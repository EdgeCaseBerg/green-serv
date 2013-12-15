Specification Testing. 
=======================================================================

The purpose of this folder is to provide a series of tests that indicate
whether or not the C API is compliant with the API specification found
[online](https://github.com/EJEHardenberg/GreenUp/blob/master/api/readme.md).

Each test is a single use case check. If a test passes, it merely emits
a . onto the command line. Should a test fail it's assertions an F will
be emitted, and the reasons for failure will be displayed to stderr. It
is advantageous then to pipe the output of stderr to its own file so it
is easy to see both the number of failures, and the results of each.

To run the specification testing, on the console type: `make spec-check`
