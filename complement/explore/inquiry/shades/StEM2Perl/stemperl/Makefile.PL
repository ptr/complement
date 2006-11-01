use 5.006;
use ExtUtils::MakeMaker;

$CC = 'c++';

# well, I use STLport and skip libstdc++ from link process
# (of cause, I can do the same without STLport; ask me how)

$beginS = `$CC -print-file-name=crtbeginS.o`;
$endS = `$CC -print-file-name=crtendS.o`;
$crtn = `$CC -print-file-name=crtn.o`;
$STLport = '$(HOME)/STLport.lab/STLport';

# my configuration assume boost, I like most of it;
# but I can live without boost

$Boost = '$(HOME)/labs/boost';

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'stem',
    VERSION_FROM      => 'lib/stem.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/stem.pm', # retrieve abstract from module
       AUTHOR         => 'Petr Ovtchenkov <ptr@island.corbina.net>') : ()),
    LIBS              => [ '-L' . $STLport . '/lib -L../../../../build/lib -L../mp/obj/gcc/shared -lpthread -lmprocessor'], # e.g., '-lm'
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    'CC'              => $CC,
    'CCFLAGS'         => '-pthread -fexceptions',
    'LD'              => '$(CC) -pthread -fexceptions -nostdlib',
    INC               => '-I. -I../../../../include -I' . $STLport . '/stlport -I' . $Boost,
    OBJECT            => "$beginS " . '$(O_FILES)' . " $endS $crtn",
    'XSOPT'           => '-C++',
    'TYPEMAPS'        => ['perlobject.map' ],
);
