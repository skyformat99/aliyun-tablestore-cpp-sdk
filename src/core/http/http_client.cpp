/* 
BSD 3-Clause License

Copyright (c) 2017, Alibaba Cloud
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "http_client.hpp"
#include <curl/curl.h>

using namespace std;

namespace aliyun {
namespace tablestore {

// HttpConfig
HttpConfig::HttpConfig()
    : mMaxConnections(5000)
    , mConnectTimeoutInMS(2000)  // 2s
    , mRequestTimeoutInMS(10000) // 30s
    , mEnableKeepAlive(true)
{
}

void HttpConfig::SetMaxConnections(int maxConnections)
{
    mMaxConnections = maxConnections;
}

int HttpConfig::GetMaxConnections() const
{
    return mMaxConnections;
}

void HttpConfig::SetConnectTimeoutInMS(int connectTimeoutInMS)
{
    mConnectTimeoutInMS = connectTimeoutInMS;
}

int HttpConfig::GetConnectTimeoutInMS() const
{
    return mConnectTimeoutInMS;
}

void HttpConfig::SetRequestTimeoutInMS(int requestTimeoutInMS)
{
    mRequestTimeoutInMS = requestTimeoutInMS;
}

int HttpConfig::GetRequestTimeoutInMS() const
{
    return mRequestTimeoutInMS;
}

void HttpConfig::SetEnableKeepAlive(bool enableKeepAlive)
{
    mEnableKeepAlive = enableKeepAlive;
}

bool HttpConfig::GetEnableKeepAlive() const
{
    return mEnableKeepAlive;
}


// HttpClient
HttpClient::HttpClient(const HttpConfig& httpConfig)
    : mHttpConfig(httpConfig)
    , mTotalConnections(0)
{
    curl_global_init(CURL_GLOBAL_ALL);
    pthread_mutex_init(&mConnectionLock, NULL);
}

HttpClient::~HttpClient()
{
    typeof(mConnectionList.begin()) iter = mConnectionList.begin();
    for (; iter != mConnectionList.end(); ++iter) {
        delete *iter;
    }
    curl_global_cleanup();
}

HttpConnection* HttpClient::GetConnection()
{
    HttpConnection* httpConn = NULL;
    pthread_mutex_lock(&mConnectionLock);
    if (mHttpConfig.GetEnableKeepAlive()) {
        if (!mConnectionList.empty()) {
            // has available connections
            httpConn = mConnectionList.front();
            mConnectionList.pop_front();
            httpConn->Reset();
        } else {
            // no available connections
            httpConn = NewConnection();
        }
    } else {
        // create a connection everytime if not keeping alive
        httpConn = NewConnection();
    }
    pthread_mutex_unlock(&mConnectionLock);
    return httpConn;
}

void HttpClient::AddConnection(HttpConnection* httpConn)
{
    pthread_mutex_lock(&mConnectionLock);
    if (mHttpConfig.GetEnableKeepAlive()) {
        mConnectionList.push_front(httpConn);
        int halfConnections = mTotalConnections / 2;
        if (mConnectionList.size() > (size_t)halfConnections) {
            // free 1/4 connections
            for (int i = 0; i < halfConnections / 2; ++i) {
                HttpConnection* tmpConn = mConnectionList.back();
                mConnectionList.pop_back();
                FreeConnection(tmpConn);
            }
        }
    } else {
        // free connection everytime if not keeping alive
        FreeConnection(httpConn);
    }
    pthread_mutex_unlock(&mConnectionLock);
}

HttpConnection* HttpClient::NewConnection()
{
    if (mTotalConnections < mHttpConfig.GetMaxConnections()) {
        HttpConnection* httpConn = new HttpConnection(
            mHttpConfig.GetConnectTimeoutInMS(), mHttpConfig.GetRequestTimeoutInMS());
        ++mTotalConnections;
        return httpConn;
    }
    return NULL;
}

void HttpClient::FreeConnection(HttpConnection* httpConn)
{
    delete httpConn;
    --mTotalConnections;
}

} // end of tablestore
} // end of aliyun