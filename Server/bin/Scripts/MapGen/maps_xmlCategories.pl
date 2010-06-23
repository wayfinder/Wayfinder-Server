#!/usr/bin/perl -w
#
# To make sure output is printed directly to stdout even if piping 
# with e.g. tee
$| = 1;
use strict;

# Print command line.
print STDERR "Command: $0";
foreach my $arg (@ARGV){
    print STDERR " $arg";
}
print STDERR "\n\n";


use Getopt::Long ':config',
    'no_auto_abbrev', # Impossible to use -s for --source.
    'no_ignore_case'  # Case sensitive options.
;    
use vars qw( $opt_help $opt_debug $opt_xml2file $opt_file2xml );
GetOptions('help|h',
           'debug|d',
           'xml2file',
           'file2xml');


# Use POIObject and perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use PerlTools;
use PerlWASPTools;
use Data::Dumper;

use File::Basename;
use XML::XPath;


sub htmlEncode {
   my $input = shift;
   $input =~ s/\&/\&amp;/;
   return $input;
}


##########################################################################
if (defined($opt_help)){
   print "Use this script dealing with POI categories,\n";
   print "to create XML from a file tree, and/or a file tree from XML\n";
   print "\n";
   print "USAGE:\n";
   print "maps_xmlCategories.pl OPTIONS INDATA\n";
   print "\n";
   print "OPTIONS:\n";
   print "-xml2file INDATA\n";
   print "Converts the xml file given as INDATA to a file tree.\n";
   print "\n";
   print "-file2xml INDATA\n";
   print "Converts the file tree pointed at by INDATA to the tree part of a ";
   print "category xml file (the category_tree XML element). The xml file is ";
   print "printed to standard out. Combine with -d.\n";
   print "\n";
   print "-d\n";
   print "Prints category name in the category tree.\n";
   print "\n";
   print "\n";
   print "\n";
   exit;
}
    



# For now handles only "]" and "[" in file names.
sub globEscape {
   my $pattern = shift;
   $pattern =~ s/\[/\\[/;
   $pattern =~ s/\]/\\]/;
   if ($pattern =~ /\[/){
      dbgprint $pattern;
   }
   return glob $pattern;
}

sub formatCatID {
   my $catID = shift;
   $catID =~ s/^0*//;
   return $catID;
}

sub printCatNodeXML {
   my $filePath = shift;
   my $level = shift;
   my $allCategories = shift;

   my $fileName = basename $filePath;

   my $catID = $fileName;
   $catID =~ s/_.*//;
   $catID = formatCatID($catID);

   my $catName = $fileName;
   $catName =~ s/^[0-9]*_//;

   my $linkAttr = ""; # false is default and needn't be in the XML.
   if ( -f $filePath ){
      $linkAttr = "cat_link=\"true\" "; # true
   }
   else {
      # Only use the real entry, not links.
      if ( defined ($allCategories->{$catID}) ){
         errprint "Double category for ID: $catID";
         exit 1;
      }
      $allCategories->{$catID} = $catName;
   }

   # Calculate indent.
   my $indent = "";
   for (my $i=0; $i<$level; $i++){
      $indent .= "   ";
   }

   if ($opt_debug){
      print "${indent}<!-- $catName -->\n";
   }
   print "${indent}<cat_node cat_id='". formatCatID($catID). "' $linkAttr>\n";

   my @files = globEscape "$filePath/*";
   foreach my $file (@files){
      printCatNodeXML($file, 
                      ($level+1),  # level in XML hierarchy
                      $allCategories );
   }
   print "${indent}</cat_node>\n";
}


sub getXMLSubElementValue {
    my $xpath = $_[0];
    my $baseNode = $_[1];
    my $elmPath = $_[2];
    
    my $expr = $elmPath . "/text()";
    my $nodeSet = $xpath->find($expr , $baseNode);

    if ( $nodeSet->size < 1 ){
        # Nothing to return.
    }
    elsif ( $nodeSet->size > 1){
        errprint("getSubElementValue Multiple results for $expr");
        exit 1;
    }
    else {
        my $node = $nodeSet->get_node(0);
        my $norecurse=1;
        return $node->toString($norecurse)
    }
}


sub getXMLAttributeValue {
   my $xpath = $_[0];
   my $node = $_[1];
   my $attrName = $_[2];

   # Build return string with attribute data from all nodes
   my $returnString = "";
   my @attributes = $node->getAttributes;

   # one attribute per node according to spec
   foreach my $attribute (@attributes){
      if ( $attribute->getName() eq $attrName ){
         return $attribute->getData();
      }
   }
   return undef;
}

sub handleXMLCatNode {
   my $xpath = shift;
   my $node = shift;
   my $filePath = shift;
   my $dirs = shift;
   my $files = shift;
   
   my $catID = getXMLAttributeValue( $xpath, $node, "cat_id" );
   my $curFilePath = "$filePath/$catID";

   my $catLink = getXMLAttributeValue( $xpath, $node, "cat_link" );

   if (! defined ($catLink) ){
      $catLink = "false";
   }
   if ( $catLink eq "true" ){
      push @{$files}, $curFilePath;
   }
   else {
      push @{$dirs}, $curFilePath;
   }
   #dbgprint "$curFilePath" . " " . $catLink;
   


   my $nodeSet = $xpath->find("cat_node", $node);
   my @catNodes = $nodeSet->get_nodelist();
   foreach my $catNode (@catNodes){
      handleXMLCatNode($xpath, $catNode, $curFilePath, $dirs, $files);
   }
   

}


sub addDescriptions {
   my $descriptions = shift;
   my $path = shift;

   my @pathParts = split "/", $path;

   my $resultPath ="";
   foreach my $part (@pathParts){
      my $desc = "";
      if ( $part ne "." ){
         $desc = $descriptions->{$part};
         $desc = "_$desc";
      }
      #dbgprint $part, $desc;
      $resultPath .= "${part}$desc/";
   }
   $resultPath =~ s/\/$//; #Strip last /-sign
   return $resultPath;
}

my %dodonaKeyByCat;
my $waspDbh;
sub initDBConn {
   $waspDbh = db_connect();
   %dodonaKeyByCat = getDodonaKeysByCategory $waspDbh;

   #foreach my $catID (keys(%dodonaKeyByCat)){
      #dbgprint "Dodona of cat $catID: $dodonaKeyByCat{$catID}";
   #}
}


sub getSearchSynonymByLangOfCat {
   my $catID = shift;
   $catID =~ s/^0*//;

   my @result;
   my $query = "SELECT langID, searchSynonym FROM POICategorySearchSynonyms " .
               "WHERE catID = $catID";
   my $sth = $waspDbh->prepare($query);
   $sth->execute();
   my $i=0;
   while (my ($lang, $syn) = $sth->fetchrow()) {
      $syn = htmlEncode($syn);
      $result[$i]{$lang} = $syn;
      $i++;
   }
   return @result;
}




# ============================================================================
# Script starting point

# ============================================================================
# File tree -> XML file
if (defined ($opt_file2xml)){
    
   dbgprint "Create XML file from file tree.\n";
   my $fileTreeRoot = $ARGV[0];

   # Used for printing content of categories element.
   my $allCategories = {};

   # Container tag
   print "<category_data>\n";
   print "\n";

   # Print category tree.
   print "<category_tree>\n";
   my @level1 = globEscape "$fileTreeRoot/*";
   foreach my $file (@level1){
      printCatNodeXML($file, 
                      1, # level in XML hierarchy
                      $allCategories);
   }

   print "</category_tree>\n";


   # Print category definitions
   initDBConn(); # filling global %dodonaKeyByCat
   my $nbrCategories = 0;
   print "\n";
   print "\n";
   print "<categories>\n";
   foreach my $catID (sort{$a <=> $b}(keys(%dodonaKeyByCat))){
       # Numeric sorting {$a <=> $b}

      my $desc = $allCategories->{$catID};
      if ( ! defined $desc ){
          $desc = $dodonaKeyByCat{$catID};
          $desc =~ s/^CAT_//;
          $desc = lc($desc);
          my @descParts = split "_", $desc;
          $desc = "";
          foreach my $descPart (@descParts){
              $desc .= ucfirst($descPart)." ";
          }
          $desc =~ s/ $//;
          $desc = ucfirst($desc);
          $desc =~ s/_/ /g;
      }
      $desc =~ s/_\[list\]//;
      $desc = htmlEncode($desc);
      my $listAttr = "";
      if ( defined ($allCategories->{$catID}) &&
           $allCategories->{$catID} =~ /_\[list\]/ ){
         $listAttr = "cat_list=\"true\" ";
      }
      print "   <category id=\"".formatCatID($catID).
          "\" desc=\"$desc\" $listAttr>\n";
      
      # fixme: this is where to write the category name translations
      # <name language='LANG'>TRANSLATION</name>
      # LANG as given in POINameLanguages.langName
      
      # write the category search synonyms
      my @synonyms = getSearchSynonymByLangOfCat($catID);
      if (defined $synonyms[0]) {
         print "\n";
      #   print Dumper(@synonyms) . "\n";
      #   foreach my $j (@synonyms) {
      #      print $j . "\n";
      #   }
      #   exit;
      }
      foreach my $line (@synonyms) {
         foreach my $langID (sort(keys( %{$line} ))) {
            my $lang = getLanguageNameFromID($waspDbh, $langID);
            print "      <search_name language='" . $lang . "'>"
               .$line->{$langID}."</search_name>\n";
         }
      }
      print "   </category>\n";
      $nbrCategories++;
   }
   print "</categories>\n";

   # End of container tag.
   print "\n";
   print "</category_data>\n";

   dbgprint "Created $nbrCategories category definitions.";
   dbgprint "Edit file manually to handle HTML encode.";
}
# ============================================================================
# XML file -> file tree
elsif  ( defined ($opt_xml2file)){

   dbgprint "Create file tree from XML.\n";
   my $xmlFileName = $ARGV[0];

   my $categoryDesc = {}; # Category descriptions by category ID

   my $xpath = XML::XPath->new(filename => $xmlFileName);

   # 1. Find the category definitions node to extract the descriptions.
   dbgprint "Reading category definitions.";
   my $expr="/category_data/categories";
   my $nodeSet = $xpath->find($expr);
   if ( defined($nodeSet) ){
      
      # Some sanity checks.
      if ( $nodeSet->size < 1 ){
         errprint "No content of category_tree node, exits!";
         exit 1;
      }
      if ( $nodeSet->size > 1){
         errprint "Should only have found one category_tree node. exits!";
         exit 1;
      }
      
      $expr="/category_data/categories/category";
      $nodeSet = $xpath->find($expr);
      my @catNodes = $nodeSet->get_nodelist();
      foreach my $catNode (@catNodes){
         my $catID = getXMLAttributeValue( $xpath, $catNode, "id" );
         if ( ! defined($catID) ){
            errprint "Missing category ID, exits";
            exit 1;
         }
         my $catDesc = getXMLAttributeValue( $xpath, $catNode, "desc" );
         if ( ! defined($catDesc) ){
            errprint "Missing category description, exits";
            exit 1;
         }
         my $catList = getXMLAttributeValue( $xpath, $catNode, "cat_list" );
         my $catListTxt = "";
         if ( defined($catList) && $catList eq "true" ){
            $catListTxt = "_[list]";
         }
         $categoryDesc->{$catID} = "$catDesc$catListTxt";
         #dbgprint "$catID $catDesc$catListTxt";
      }
   }
   else {
      errprint "Could not find the \"categories\" node, exits";
      exit 1;
   }

   # 2. Find the tree container node.
   dbgprint "Reading category tree.";
   $expr="/category_data/category_tree";
   $nodeSet = $xpath->find($expr);
   if ( defined $nodeSet ){

      # Some sanity checks.
      if ( $nodeSet->size < 1 ){
         errprint "No content of category_tree node, exits!";
         exit 1;
      }
      if ( $nodeSet->size > 1){
         errprint "Should only have found one category_tree node. exits!";
         exit 1;
      }      

      my $baseNode = $nodeSet->get_node(0);
      $nodeSet = $xpath->find("cat_node", $baseNode);
      
      my $files = []; # only category IDs.
      my $dirs = [];  # only category IDs.

      my @catNodes = $nodeSet->get_nodelist();
      foreach my $node (@catNodes){
         handleXMLCatNode($xpath, 
                          $node, 
                          ".",
                          $dirs,
                          $files);
      }

      dbgprint "Creating file tree, printing dirs";
      foreach my $dir (@{$dirs}) {
         $dir = addDescriptions($categoryDesc, $dir);
         #dbgprint "$dir";
         `mkdir $dir`;
      }
      dbgprint "Creating file tree, printing files";
      foreach my $file (@{$files}) {
         $file = addDescriptions($categoryDesc, $file);
         #dbgprint "$file";
         `touch $file`;
      }
   }
   else {
      errprint "Could not find the \"category_tree\" node, exits";
      exit 1;
   }
}
exit;
