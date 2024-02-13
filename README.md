# C++Builder Extras

A collection of tools for working in the Borland/Embarcadero ecosystem to smooth down one or more of the sharp edges. While these tools were designed with a particular environment and style in mind, they should be compatible with (almost) any compiler across any C++ standard.

To use, add the include directory to your compiler's include path. The tools in this repo exist in `namespace dp`.

## List of Features

A wiki is provided with a full writeup of each feature. A short summary of the included features are:

* Contracts - Function contract annotations designed following a similar pattern to the current proposed contract MVP for C++26 (P2900)
* Convert - A generic type conversion function which converts between built-in, standard library, and VCL types.
* Defer - A tool to defer the evaluation of certain expressions until the exit of the current scope.