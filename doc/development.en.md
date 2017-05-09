# Development Guideline

## For platform developers

A platform developer develops or maintains support of a specific platform, such as centos.

Here is a workflow:
1. Makes a directory under `docker/`.
   * Name of the directory must concatenate name of the platform and its major release version, e.g., centos7.
1. Creates a dockerfile under the directory, so that both `scons` and `scons mode=debug` pass.
1. Passes all tests on a container created by the dockerfile.
   See [Testing](#testing) for details.

## For feature developers

Before check in a new feature, developers must make sure
1. enough tests (including both unit tests and function tests) are added.
1. All tests over all platforms are passed. See [Testing](#testing) for details.
These rules can be violated only when there are strong enough reasons.

## Testing

### Prerequisites

To run function tests, the following must be prepared on [Alibaba Cloud Computing](https://intl.aliyun.com):
* An account of Alibaba Cloud Computing. So you have both AccessKey id and AccessKey secret.
* An instance of TableStore. So you have a name and its endpoint.
Fill them in `test/functiontest/config.toml`.

Notice, NEVER check in `config.toml` with your account or instance.

### Directory tree

```
test-+
     +-testa
     +-functiontest
     +-unittest-+
                +-util
                +-core
```
* `testa` is a copy of [Testa](https://github.com/TimeExceed/testa),
  which is a simple testing framework based on a simple and unique testing philosophy.

### Running tests

After compilation, testing binaries will be placed in `build/$MODE/bin`.
As far as this document written, there are `core_functiontest`, `core_unittest` and `util_unittest`.
They can be executed by testa, e.g.,
```sh
# test/testa/runtests.py --lang test/testa/lang.config build/$MODE/bin/core_functiontest build/$MODE/bin/core_unittest build/$MODE/bin/util_unittest
1/97 pass: build/release/bin/core_unittest/Action_pp costs 0:00:00.006556 secs
2/97 pass: build/release/bin/core_unittest/AttributeValue_blob costs 0:00:00.012437 secs
...
97/97 pass: build/release/bin/util_unittest/MonotonicTime_now costs 0:00:01.003448 secs

0 failed
```
If any one of them are failed, you can check their outputs.
For example, if `build/release/bin/util_unittest/MonotonicTime_now` is failed,
you can find its stdout and stderr in `test_results/build/release/bin/util_unittest/MonotonicTime_now.out` and `test_results/build/release/bin/util_unittest/MonotonicTime_now.err` respectively.
See `--help` of `runtests.py` for more informations.
