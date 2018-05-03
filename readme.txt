./config && make
./config shared && make

./config -DOPENSSL_WITH_INTEL -I../../ipp/include/ -L../ipp/lib/x86/ -lippcp -lippcore && make
./config shared -DOPENSSL_WITH_INTEL -I../../ipp/include/ -L./ipp/lib/x86/ -L../ipp/lib/x86/ -L../../ipp/lib/x86/ -lippcp -lippcore && make

./config -DOPENSSL_WITH_INTEL -I../../ipp/include/ -L../ipp/lib/x64/ -lippcp -lippcore && make
./config shared -DOPENSSL_WITH_INTEL -I../../ipp/include/ -L./ipp/lib/x64/ -L../ipp/lib/x64/ -L../../ipp/lib/x64/ -lippcp -lippcore && make


# Visual Studio Command Prompt
perl Configure VC-WIN32 && ms\do_nasm && nmake -f ms\nt.mak
perl Configure VC-WIN32 && ms\do_nasm && nmake -f ms\ntdll.mak

perl Configure VC-WIN32 -DOPENSSL_WITH_INTEL -I../../ipp/include/ -L./ipp/lib/x86/ && ms\do_nasm && nmake -f ms\nt.mak
perl Configure VC-WIN32 -DOPENSSL_WITH_INTEL -I../../ipp/include/ -L./ipp/lib/x86/ && ms\do_nasm && nmake -f ms\ntdll.mak

# Visual Studio x64 Win64 Command Prompt
perl Configure VC-WIN64A && ms\do_win64a && nmake -f ms\nt.mak
perl Configure VC-WIN64A && ms\do_win64a && nmake -f ms\ntdll.mak

perl Configure VC-WIN64A -DOPENSSL_WITH_INTEL -I../../ipp/include/ -L./ipp/lib/x64/ && ms\do_win64a && nmake -f ms\nt.mak
perl Configure VC-WIN64A -DOPENSSL_WITH_INTEL -I../../ipp/include/ -L./ipp/lib/x64/ && ms\do_win64a && nmake -f ms\ntdll.mak
