use 5.006;
use ExtUtils::MakeMaker;

$CC = 'c++ -pthread';
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'stem::NetTransport',
    VERSION_FROM      => 'lib/stem/NetTransport.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/stem/NetTransport.pm', # retrieve abstract from module
       AUTHOR         => 'Petr Ovtchenkov <ptr@island.corbina.net>') : ()),
    LIBS              => ['-L/export/home/ptr/STLport.lab/STLport/lib -L../../../../build/lib -lxmt -lsockios -lstem'], # e.g., '-lm'
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    'CC'              => $CC,
    'LD'              => '$(CC) -nostdlib',
    INC               => '-I. -I../../../../include -I$(HOME)/STLport.lab/STLport/stlport -I$(HOME)/labs/boost', # e.g., '-I. -I/usr/include/other'
	# Un-comment this if you add C files to link with later:
    # OBJECT            => '$(O_FILES)', # link all the C files too
    OBJECT            => '/opt/gcc-4.1.1/lib/gcc/i686-pc-linux-gnu/4.1.1/crtbeginS.o $(O_FILES) /opt/gcc-4.1.1/lib/gcc/i686-pc-linux-gnu/4.1.1/crtendS.o /usr/lib/crtn.o',
    'XSOPT'           => '-C++',
    'TYPEMAPS'        => ['perlobject.map' ],
);
