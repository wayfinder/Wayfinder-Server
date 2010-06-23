def create_tool(bld, target):
   """Creates and returns a default target used in Server/Tools"""

   prog = bld.new_task_gen( 'cxx', 'program' )
   prog.source = bld.glob('*.cpp')
   prog.includes = '../include'
   prog.target = target
   prog.install_path = None
   prog.uselib_local = 'ServersShared Shared'
   prog.uselib = 'SHARED SERVERSSHARED'

   return prog

def create_server_tool(bld, target):
   """Creates and returns a target used in Server/Server/Tools"""

   prog = bld.new_task_gen( 'cxx', 'program' )
   prog.source = bld.glob('*.cpp')
   prog.includes = '../include'
   prog.target = target
   prog.install_path = None
   prog.uselib_local = 'ServersSharedNGP ServersServers ServersShared Shared'
   prog.uselib = 'SHARED SERVER SERVERSSHARED'

   return prog

def create_tool_gtk(bld, target):
   """Creates and returns a gtk target used in Server/Tools"""
   prog = create_tool(bld, target)
   prog.uselib_local += ' MC2Gtk'
   prog.uselib += ' GTK'

   return prog

def create_server(bld, target):
   """Creates and returns a server target"""

   prog = bld.new_task_gen( 'cxx', 'program' )
   prog.source = bld.glob("*.cpp")
   prog.includes = '../include'
   prog.target = target
   prog.install_path = None
   prog.uselib_local = 'ServersSharedNGP ServersServers ServersShared ServersSharedNet \
    ServersSharedDrawing ServersSharedGfx ServersSharedXML \
ServersSharedCommon ServersSharedItems ServersSharedDatabase Shared SharedNet \
   SharedFileSystem SharedUtility'
   prog.defines = 'USE_SSL USE_XML'
   prog.uselib='SHARED SERVER DRAWING SERVERSSHARED'

   return prog

def create_module(bld, target):
   """Creates and returns a module target"""

   prog = bld.new_task_gen( 'cxx', 'program' )
   prog.source = bld.glob("*.cpp")
   prog.includes = '../include'
   prog.target = target
   prog.install_path = None
   prog.uselib_local = 'Module ServersShared ServersSharedNet \
    ServersSharedDrawing ServersSharedGfx ServersSharedXML \
ServersSharedCommon ServersSharedItems ServersSharedDatabase Shared SharedNet'
   prog.defines = 'USE_XML'
   prog.uselib='MODULE SHARED SERVERSSHARED'

   return prog
