.. _install:

Installation
============

Scapi is simple enough to install, the installation varies on different operating systems. Scapi currently supports Linux_ and Windows_.

.. _Linux:  

Installing LibSCAPI - Linux
---------------------------
The following explains how to install libscapi on Ubuntu. For other Linux variants it should work as well with the appropriate adjustments.

Prerequisites
~~~~~~~~~~~~~

Update and install git, gcc, gmp, and open ssl. On Ubuntu environment is should look like: ::

  $ sudo apt-get update
  $ sudo apt-get install -y git build-essential
  $ sudo apt-get install -y libssl-ocaml-dev libssl-dev 
  $ sudo apt-get install -y libgmp3-dev
  
Download and install boost 1.71 (the last step might take some time, patience): ::
  
  $ wget https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.bz2
  $ tar -xf boost_1_71_0.tar.bz2
  $ cd boost_1_71_0
  $ ./bootstrap.sh --prefix=/usr && ./b2 stage threading=multi link=shared
  $ sudo ./b2 install threading=multi link=shared

More details about boost here: http://www.boost.org/doc/libs/1_71_0/more/getting_started/unix-variants.html  

Building libscapi and publishing libs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Download and build libscapi: ::

  $ cd ~
  $ git clone https://github.com/cryptobiu/libscapi.git
  $ cd libscapi
  $ make
  
Publish new libs: ::
  
  $ sudo ldconfig ~/boost_1_71_0/stage/lib/ ~/libscapi/install/lib/

Building and Running the Tests
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to build and run tests: ::

  $ cd ~/libscapi/tests
  $ ./ScapiTests

Samples
~~~~~~~

Build the samples program: ::

  $ cd ~/libscapi/examples
  $ cmake . && make

In order to see all available samples: ::

 $ ./libscapi_example
 
In order to run simple examples (dlog or sha1): ::

 $ ./libscapi_example dlog 
 $ ./libscapi_example sha1
 
You should get some print outs if everything works well.

In order to run the CommExample. Open two terminals. In the first run: ::
 
 $ ./libscapi_example comm 1 Comm/CommConfig.txt
 
And in the other run: ::

 $ ./libscapi_example comm 2 Comm/CommConfig.txt

In order to run Semi-honset YAO, run in the first terminal: ::

  $ ./libscapi_example yao 1 Yao/YaoConfig.txt

And in the second: ::
  
  $ ./libscapi_example yao 2 Yao/YaoConfig.txt
  
Finally in order to run the Sigma example - in the first terminal run: ::

  $ ./libscapi_example sigma 1 SigmaPrototocls/SigmaConfig.txt

And in the second terminal: ::

 $ ./libscapi_example sigma 1 SigmaPrototocls/SigmaConfig.txt

You can edit the config file in order to play with the different params in all examples.

.. _Windows:

Installing LibSCAPI - Windows
-----------------------------

Installing scapi on windows will require git client and Visual Studio IDE. We tested it with VS2015.

Prerequisites: 

1. Download and install open ssl for windows: https://slproweb.com/products/Win32OpenSSL.html (choose 64bit not light)
2. Download and install boost binaries for windos: https://sourceforge.net/projects/boost/files/boost-binaries/1.64.0/ choose 64 bit version 14

The windows solutions assume that boost is installed at ``C:\local\boost_1_64_0`` and that OpenSSL at: ``C:\OpenSSL-Win64``


Pull libscapi from GitHub. For convenient we will assume that libscapi is located at: ``c:\code\scapi\libscapi```. If it is located somewhere eles then the following paths should be adjusted accrodingly.

1. Build Miracl for windows 64:
	A. Open solution MiraclWin64.sln at: ``C:\code\libscapi\lib\MiraclCompilation``
	B. Build the solution once for debug and once for release
2. Build OTExtension for window 64:
	A. Open solution OTExtension.sln at ``C:\code\libscapi\lib\OTExtension\Win64-sln``
	B. Build solution once for debug and once for release
3. Build GarbledCircuit project
	A. Open solution ScGarbledCircuitWin64.sln at ``C:\code\libscapi\lib\ScGarbledCircuit\ScGarbledCircuitWin64``
	B. Build solution once for debug and once for release
4. Build the NTL solution:
  	A. Open solution NTL-WIN64.sln at ``C:\code\libscapi\lib\NTL\windows\NTL-WIN64``
  	B. Build solution once for debug and once for release
5. Build Scapi Solution including examples and test:
	A. Open solution ScapiCpp.sln at ``C:\code\libscapi\windows-solutions\scapi-sln``
	B. Build solution once for debug and once for release - (as needed)
	
6. Run tests.
	A. Go to ``C:\code\libscapi\windows-solutions\scapi-sln\x64\debug``
	B. run ./scapi_tests.exe and make sure all is green
7. Run example:
	A. open two terminals
	B. in both of them go to: ``C:\code\libscapi\windows-solutions\scapi-sln\x64\debug``
	C. To see available samples run ``libscapi_examples.exe``
	D. Follow instruction of how to run the different samples as exaplained in the linux section
	E. You can edit the different config file to play with the paramaters

