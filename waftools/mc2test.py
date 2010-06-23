def create_sources(bld, prefix, globstr, exclude_this):
    """ Creates a sources list from prefix + globstr matching pattern and
    excluding the "exclude_this" from the matching.
    """
    sources = bld.glob( prefix + globstr )
    sources.remove(exclude_this)
    sources = [ prefix + x for x in sources ]
    return sources

def unit_test(bld, target, sources, uselib_local = None, uselib = None):
    """ Creates a unit test task for waf build.  """
    test = bld.new_task_gen( 'cxx', 'program' )
    test.obj_ext = '_2.o'
    test.source = sources
    test.includes = '../include'
    test.target = target
    test.install_path = None
    test.uselib_local = uselib_local + ' SharedUnitTest'
    test.uselib = uselib
    test.unit_test = 1
    return test
