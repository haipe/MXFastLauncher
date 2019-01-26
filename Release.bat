

set root_src_dir=%~dp0
set root_dst_dir=%~dp0

echo %root_src_dir%\
echo %root_dst_dir%\

cd %root_src_dir%\

set bin_release_dir=%root_src_dir%\Bin\Release\
set bin_debug_dir=%root_src_dir%\Bin\Debug\

mkdir %bin_release_dir%
mkdir %bin_release_dir%Skin\
rem copy ---------------------------------------------------------------

del %bin_release_dir%\Skin\*.*  /q
xcopy %bin_debug_dir%\Skin %bin_release_dir%\Skin /e /h /y 



pause