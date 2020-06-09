# Project README file

This is **YOUR** Readme file.

## Project Description
We will manually review your file looking for:

- A summary description of your project design.  If you wish to use grapics, please simply use a URL to point to a JPG or PNG file that we can review

- Any additional observations that you have about what you've done. Examples:
	- __What created problems for you?__
	TODO
	- __What tests would you have added to the test suite?__
	TODO
	- __If you were going to do this project again, how would you improve it?__
	TODO
	- __If you didn't think something was clear in the documentation, what would you write instead?__
	TODO

## Known Bugs/Issues/Limitations

__Please tell us what you know doesn't work in this submission__
TODO
## References

__Please include references to any external materials that you used in your project__
Connection refused error : i had to run "sudo rpc_bind " to pass this error

RPC_CANTENCODEARGS:
1. I tried malloc JPEG_OUT and its members inside client  -- DIDN"T work
2. Rewrote the .x with different address and generated rpcgen == DIDN"T work
3. i defined my data IN and OUT buffer in .x as

typedef MAX_LEN 1024
struct JPEG_IN {
 opaque buffer<MAX_LEN> ;
};

it should be infinite allocation and not as described in the class video.






