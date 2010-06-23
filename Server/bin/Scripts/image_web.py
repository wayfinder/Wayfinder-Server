#!/usr/bin/env python

needed_endings = [ '.png' ]
interesting_endings = [ '.png', '.svg', '_rc.png' ]
uninteresting_endings = [ '.svg', '_rc.png', '_old.png' ]

def endswith(s, end):
   return s[-len(end):] == end

def add_table_data(html, inhalt):
   html = html + "   <td>" + inhalt + "</td>\n"
   return html

def add_image(page, filename):
   toadd = "<tr>\n"
   for i in interesting_endings:
      curfile = filename + i
      toadd = add_table_data( toadd, curfile )
      try:
         f = open(curfile)
         file_exists = 1
         f.close()
      except IOError:
         file_exists = 0

      if file_exists:
         if not endswith( curfile, '.svg' ):
            toadd = add_table_data( toadd, '<image src="'+curfile+'">' )
         else:
            x = '<object data="'+curfile+'" type="image/svg+xml"></object>'
            toadd = add_table_data( toadd, x)
      else:
         toadd = add_table_data( toadd, 'NEJ' )
    
   toadd = toadd + "</tr>\n"

   return page + toadd

def keep_needed(li):
   new_li = []
   for i in li:
      for j in needed_endings:
         # Endswith
         if i[-len(j):] == j:
            new_li.append(i)
            break
   return new_li

def keep_base(li):
  new_li = []
  for i in li:
     add = 1
     for j in uninteresting_endings:
         # Endswith
         if i[-len(j):] == j:
            add = 0
            break
     # Remove .png
     if add:
        import re
        image_base = re.sub( r'\.png', '', i )
        new_li.append(image_base)
     
  return new_li


def main():
   page = "<html><head><title>Images</title></head><body>"
   page = page + '<table border="1">'
   import sys
   import dircache
   l = keep_base( keep_needed( dircache.opendir('.') ) )
   for i in l:
      page = add_image(page, i)
   page = page + "</table></body></html>"
   print page

if __name__ == "__main__":
    main()
