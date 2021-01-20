# SC Compiler

This is a simple compiler for SC. (Small/Simple C or Smart Contract)

SC is a C-like language for Omega Chain Smart Contracts. The compiler will translate SC program into 
OVM (Omega Virtual Machine) assembly code. See http://omegasuite.org/omega/files/ovmasmspec.html for
assembly spec and http://omegasuite.org/omega/files/ovmbcspec.html for VM code spec.

To further translate assembly code to VM code, use http://omegasuite.org/omega/assemble.php.

Win 64 excutable is include for your convenience. To compile, run:

sc.exe &lt;source file name&gt;

Make sure the file buildin.c is in your source code directory.

The compiler will generate two files with .asm and .abi suffixes. The one with .asm suffix contains
assembly code. Copy and paste it to http://omegasuite.org/omega/assemble.php, will generate VM code
in both hex and text formats. The file with .abi suffix is a JSON file to be used by other smart
contract for importation. Before it could be imported, you need to modify "address" in the JSON file
to the actual address of the depolyed smart contract (without the 0x88 NetID byte).

Differences Between ANSI C and SC

1. The entire smart contract code must be in one file.
2. There is no main().
3. The constructor() function is smart contract constructor. It will be executed once at the time of
smart contract creation and discarded thereafter.
4. The _() function is a fall back function. It will be called when no public function matches call
script.
5. Functions with 'public' property are callable from outside.
6. There is only one form of loop: while.
7. There is no ++ or --.
8. Function init() (if defined) is a lib initialization function. It will be executed once when a contract
is loaded as a lib.
9. There is no float and bool data types.
10. Int types are: char/uchar, short/ushort, int/uint, long/ulong, big. And they are 8-bit, 16-bit, 32-bit,
64-bit, 256-bit respectively. The ones start with u are unsigned versions.
11. Syntax for variable declaration is:
	type id-list;
    Thus by "int * a, b;", both a and a are int pointers. (not like C).
12. Can not initialize variables at the time of declaration. Thus int a = 1; is illegal.

13. You may call another contract. To call it, you need to import the callee first using import statement:
	import &lt;contract name&gt;;
    Compiler will find corresponding &lt;contract name&gt;.abi file to imports its interface. Note: make sure "address"
    is correct.

14. There are two ways to call another contract's public function. As lib or as independent contract. To call
it as lib, a libload("&lt;contract name&gt;") call must be executed at least once before call to the function. Multiple
libload call is OK, but only the first has effect. Then you may call another contract's public function as follows:
	&lt;contract name&gt;::func(parameters);
    In this type call, func uses present contract's persistent storage space and memory space as if the code
is the present contract's own.
    Another way call to another contract's public function is:
	&lt;contract name&gt;::execute(parameters);
    In this type call, func uses callee contract's storage space amd indenpendent memory space, thus passing
a pointer to the contract will generate an error when the pointer is used. No need to call libload before execute().

15. Call Contracts in transactions. To call contract in a transaction, you need to encode the call into a script.
The script begings with the contract's 21 byte address (including 0x88 NetID), followed by a little endian 32-bit
number. This number is a public function's ID (defined in .abi file). And then parameters. In SC, parameters are always
passed by value and as 64-bit numbers. If you need to pass long data such as a string or a struct, you may pass
an offset (the function should interpret it as a pointer) and the actual data folling function parameters. Add 12
to the offset. E.g., if the prototype of a public function is:
	public func(short n, char * s);
    If you want to execute func(125, "abcdefghijklmnopqrstuvwxyz") in a transaction. Then the output script should be:

    |--- 21 byte contract address ---|- 4 bytes func ID -|-- 125 (8 bytes) --|-- 28 (8 bytes) --|abcdefghijklmnopqrstuvwxyz
	
    For parameter s, you should use value 28 as staring address of "abcdefghijklmnopqrstuvwxyz" because
	28 = 12 (don't ask why) + 8 (length of parameter n) + 8 (length of parameter s).

16. For anything else, see the examples.
