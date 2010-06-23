
import Task
import Constants
import UnitTest
import os
import sys
import Utils

sys.path.insert(0,os.getcwd())

# This will be the default value for the algorithm type
Task.algotype = Constants.JOBCONTROL

srcdir = '.'
blddir = 'output'

def set_options(opt):
    opt.tool_options('compiler_cxx')
    opt.tool_options('boost')

    opt.add_option('--algotype', action='store', default = Task.algotype,
                   help="Give the type of job control algorithm you want to \
                   use: \nChose between NORMAL, JOBCONTROL, MAXPARALLEL\
                   [Default: %s]"%Task.algotype, dest='algo_type')

    opt.add_option('--distcc', action='store_true', default=False,
                   help='Enable compilation with distcc when configuring');
    opt.add_option('--docs', action='store_true', default=False,
                   help='Compile documentation, nothing else.')
    opt.add_option('--single', action='store_true', default=False,
                   help='Enable running on localhost only no multicast, used when configuring.')


def configure(conf):
    # Setup distcc compilation if --distcc was used
    import Options
    if Options.options.distcc:
        # Make sure we have distcc installed and
        # set the new compiler to it
        conf.find_program('distcc')
        conf.env['CXX'] = 'distcc g++'
        # Need 32 bit compilation for this because the other
        # compilation machines are 64
        # FIXME: Add check if local is 32 or 64 bit before setting 32 here
        conf.env.append_value( 'CXXFLAGS', '-m32' )

    conf.check_tool('compiler_cxx')
    conf.check_tool('tex')
    conf.check_tool('boost')
    conf.env['LIB_BOOST_REGEX'] = 'boost_regex'

    # Set up single version if --single aregument was used
    if Options.options.single:
        conf.env.append_value( 'CXXFLAGS', '-DSINGLE_VERSION' )

    # Common setup
    conf.env.append_value( 'CXXFLAGS', '-DMC2_SYSTEM' )
    conf.env.append_value( 'CXXFLAGS', '-DARCH_OS_LINUX' )
    conf.env.append_value( 'CXXFLAGS', '-D__linux' )
    conf.env.append_value( 'CXXFLAGS', '-DDEBUG_LEVEL_1')
    conf.env.append_value( 'CXXFLAGS', '-D_REENTRANT=1')
    conf.env.append_value( 'CXXFLAGS', '-DUSE_XML' )
    conf.env.append_value( 'CXXFLAGS', '-DUSE_SSL' )
    conf.env.append_value( 'CXXFLAGS', '-DZLIB' )
    conf.env.append_value( 'CXXFLAGS', '-DHAVE_ZLIB=1')
    conf.env.append_value( 'CXXFLAGS', '-g' )
    conf.env.append_value( 'CXXFLAGS', '-pipe' )
    conf.env.append_value( 'CXXFLAGS', '-Wall' )
    conf.env.append_value( 'CXXFLAGS', '-Werror' )
    conf.env.append_value( 'CXXFLAGS', '-Wpointer-arith' )
    conf.env.append_value( 'CXXFLAGS', '-Wcast-align' )
    conf.env.append_value( 'CXXFLAGS', '-Woverloaded-virtual' )

    # Xerces configuration
    conf.env[ 'CXXFLAGS_XERCES' ] = '-I/usr/include/xercesc'
    conf.env[ 'LINKFLAGS_XERCES' ] = '-lxerces-c'

    # Tecla, for read line support
    conf.env[ 'LIB_TECLA' ] = 'tecla'

    # Check sub configurations

    conf.sub_config('Shared/')
    conf.sub_config('Server/Shared/')
    conf.sub_config('Server/Servers/src')
    conf.sub_config('Server/Modules/src')
    conf.sub_config('Server/MapGen/')
    conf.sub_config('Server/Tools' )


def init():
    import Options
    Task.algotype = Options.options.algo_type

def build(bld):
    import Options
    if not Options.options.docs:
       # Do not add any document dirs here
       bld.add_subdirs( 'Shared' )
       bld.add_subdirs( 'Server/Shared' )
       bld.add_subdirs( 'Server/Servers' )
       bld.add_subdirs( 'Server/Modules' )
       bld.add_subdirs( 'Server/MapGen' )
       bld.add_subdirs( 'Server/Tools' )
    elif bld.env.LATEX:
       # only build docs
       bld.add_subdirs( 'docs/Design' )

def do_unit_test():
    # Unit tests are run when "check" target is used
    ut = UnitTest.unit_test()
    ut.change_to_testfile_dir = True
    ut.run()
    ut.print_results()

# This function is run with 'waf check' for waf 1.5.6
def check(context):
    do_unit_test()

def shutdown():
    # In version 1.5.6 and above, the check(context function
    # is used for unit testing. But in version 1.5.3 ( and possible
    # version in between ) the unit test is done in shutdown

    max_val = int('1.5.6'.replace('.', '0'), 16)

    if Utils.HEXVERSION < max_val:
        do_unit_test()


