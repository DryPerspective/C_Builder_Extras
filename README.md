# C++Builder Extras

A collection of tools for working in the Borland/Embarcadero ecosystem to smooth down one or more of the sharp edges. While these tools were designed with a particular environment and style in mind, they should be compatible with (almost) any compiler across any C++ standard.

Tools are provided in two "epoch" standards - one for C++98 and one for C++17. This mirrors the two language standards supported by C++Builder, which was the original IDE these tools were built for. The library is designed in such a way that the user does not need to actively choose which standard they use - the correct header will be automatically selected for them. This does of course mean that ABI may change between language standards, but C++Builder breaks ABI on every update so for the intended use-case this was a minimal concern.
To use, add the include directory to your compiler's include path. The tools in this repo exist in `namespace dp`.

## List of Features

A wiki is provided with a full writeup of each feature. A short summary of the included features are:

* Contracts - Function contract annotations designed following a similar pattern to the current proposed contract MVP for C++26 (P2900)
* Convert - A generic type conversion function which converts between built-in, standard library, and VCL types.
* Defer - A tool to defer the evaluation of certain expressions until the exit of the current scope.