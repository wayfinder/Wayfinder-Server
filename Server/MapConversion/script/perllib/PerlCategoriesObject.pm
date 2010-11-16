#!/usr/bin/perl -w
#
#
# Class that represents all categories.

package PerlCategoriesObject;

# Todo: get BASEGENFILESPATH into the use-lib- string possible?
use lib ".";
use PerlTools; # for dbgprint etc

use strict;
use XML::XPath;

# Static constants
#my $fieldSep = "<¡>";   # Separator in array strings

#########################################################


# Create empty POI object, possible to set id

sub dprint1 {
   dbgprint "[CAT]", @_;
}

sub dprint2 {
   #dbgprint "[CAT]", @_;
}

sub newObject {

   #if ( defined( $_[1]) ) {
   #   $poiID = $_[1];
   #}
   my $self = {};

   # Hash describing a category node
   # catNode->{'ID'}: The ID of this category
   # catNode->{'link'}: 1 if this node represens a link, 0 otherwise.
   # catNode->{'childs'}: An array ref with all child nodes of this node as 
   #                      hash refs.
   @{$self->{m_categoryTree}} = (); # Array storing catNode hash refs.
   
   # catDef->{'ID'}: The ID of this category
   # catDef->{'desc'}: The description of this category
   # catDef->{'list'}: 1 if this category is part of the standard category list 
   #                   used when searching, 0 otherwise.
   # catDef->{'names'}: Hash with names by language name.
   $self->{m_categoryDefs} = (); # Array with catDef hash refs.

   #undef $self->{m_source};
   return bless $self;
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

# Returns 1 if the category if found in the tree, otherwise, 0.
sub catInTree {
   my $self = shift;
   my $catID = shift; # the category ID to look for in the tree.
   my $catNodeRef2 = shift; #

   if (!defined $catNodeRef2) {
      foreach my $catNodeRef1 (@{$self->{m_categoryTree}}){
         if ($catNodeRef1->{'ID'} == $catID){
            return 1; # Found the category ID in the tree.
         }
         foreach my $childNode (@{$catNodeRef1->{'childs'}}) {
            if ($self->catInTree($catID, $childNode)) {
               return 1;
            }
         }
      }
   } else {
      if ($catNodeRef2->{'ID'} == $catID){
         return 1; # Found the category ID in the tree.
      }
      foreach my $childNode (@{$catNodeRef2->{'childs'}}) {
         if ($self->catInTree($catID, $childNode)) {
            return 1;
         }
      }
   }
   return 0;
}

sub handleXMLCatNode {
   my $self = shift;

   my $xpath = shift;
   my $node = shift;
   my $parentNode = shift; # Hash ref.

   my $currentNode = {};

   # Read ID
   $currentNode->{'ID'} = getXMLAttributeValue( $xpath, $node, "cat_id" );
   $currentNode->{'ID'} =~ s/^0*//;
   
   # Read link bool
   my $catLink = getXMLAttributeValue( $xpath, $node, "cat_link" );
   if (! defined ($catLink) ){
      $currentNode->{'link'} = 0;
   }
   elsif ( $catLink eq "true" ){
      $currentNode->{'link'} = 1;
   }
   else {
      $currentNode->{'link'} = 1;
   }
   if ( ! defined $parentNode ){
      push @{$self->{m_categoryTree}}, $currentNode;
   }
   else {
      push @{$parentNode->{'childs'}}, $currentNode;
   }

   my $nodeSet = $xpath->find("cat_node", $node);
   my @catNodes = $nodeSet->get_nodelist();
   foreach my $catNode (@catNodes){
      $self->handleXMLCatNode($xpath, $catNode, $currentNode);
   }
}


sub getParents {
   my $self = shift;
   my $catID = shift;     # The category ID to look for parents of.
   my $parentIDs = shift; # Outparameter. Hash with all parent nodes.
   my $catNode = shift;   # Only for recursive calls.

   my $result = 0;

   if ( ! defined($catNode) ){
      # We're at toplevel.
      foreach my $locCatNode (@{$self->{m_categoryTree}}){
         if ($self->getParents($catID, $parentIDs, $locCatNode)){
            $result = 1;
         }
      }
   }
   else {
      # We're in the tree
      if ( $catNode->{'ID'} == $catID ){
         $result = 1;
      }
      else {
         foreach my $childCatNode (@{$catNode->{'childs'}}){
            if ($self->getParents($catID, $parentIDs, $childCatNode)){
               # This node is parent to some node matching the catID
               $result = 1;
               $parentIDs->{$catNode->{'ID'}} = 1;
            }
         }
      }
   }
   return $result;
}

sub eliminateHigherLevelDoubles {
   my $self = shift;
   my $catIDs = shift; # Outparamter. Array ref wiht all category IDs to check.
                       # When the method is done, this array only includes the
                       # lowest level category IDs among the originally included
                       # category IDs. That is, no category ID is a parent or a
                       # parents parent to any of the others in the array.

   # Stores all category IDs found to be a higher level double.
   my %higherLevelDoubles = ();

   
   foreach my $catID1 (@{$catIDs}){
      my $double = 0;
      my $catID1Parents = {};
      $self->getParents($catID1, $catID1Parents);
      
      # Check if any of the other category IDs is one of this category's parents.
      foreach my $catID2 (@{$catIDs}){
         #dbgprint "Checking $catID2";
         if (defined $catID1Parents->{$catID2}){
            # catID2 is a parent to $catID1
            $higherLevelDoubles{$catID2} = 1;
            #dbgprint "$catID2 is higher";
         }
      }
   }
   
   # Clear out higher IDs from the result out paramter.
   for (my $i = scalar(@{$catIDs})-1; $i >=0; $i--){   
      if ( defined $higherLevelDoubles{$catIDs->[$i]} ){
         splice @{$catIDs}, $i, 1;
         #dbgprint "Spliced:", @{$catIDs};
      }
   }
}


sub printCatNode {
   my $catNode = shift; # Hash ref
   my $indent = shift;
   $indent.="   ";

   dprint1 "${indent}ID: $catNode->{'ID'}, link(".$catNode->{'link'}.")";
   foreach my $childCatNode (@{$catNode->{'childs'}}){
      printCatNode($childCatNode, $indent);
   }
}

sub printCatDef {
   my $catDef = shift; # Hash ref
   
   dprint1 "ID: $catDef->{'ID'}, desc: $catDef->{'desc'}, list(".
       $catDef->{'list'}.") names:";
   foreach my $lang (sort(keys(%{$catDef->{'names'}}))){
      dprint1 "   $lang: ${$catDef->{'names'}}{$lang}";
   }

   # catDef->{'ID'}: The ID of this category
   # catDef->{'desc'}: The description of this category
   # catDef->{'list'}: 1 if this category is part of the standard category list 
   #                   used when searching, 0 otherwise.
   # catDef->{'names'}: Hash with names by language name.

}



sub loadFromXML {
   my $self = $_[0];
   my $xmlFileName = $_[1]; # Name and path of the XML file.

   dprint1 "Create file tree from XML file: $xmlFileName";
   my $xpath = XML::XPath->new(filename => $xmlFileName);
   
   #XXX my $categoryDesc = {}; # Category descriptions by category ID
   
   
   # 1. Find the category definitions node to extract the descriptions.
   dprint1 "Reading category definitions.";
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
      foreach my $catXmlNode (@catNodes){
         my $currCatDef = {};

         $currCatDef->{'ID'} = getXMLAttributeValue( $xpath, $catXmlNode, "id" );
         if ( ! defined($currCatDef->{'ID'}) ){
            errprint "Missing category ID, exits";
            exit 1;
         }
         $currCatDef->{'desc'} = getXMLAttributeValue( $xpath, $catXmlNode, "desc" );
         if ( ! defined($currCatDef->{'desc'}) ){
            errprint "Missing category description, exits";
            exit 1;
         }
         my $catList = getXMLAttributeValue( $xpath, $catXmlNode, "cat_list" );
         if ( defined($catList) && $catList eq "true" ){
            $currCatDef->{'list'} = 1;
         }
         else {
            $currCatDef->{'list'} = 0;
         }

         my $nameNodeSet = $xpath->find("name", $catXmlNode);
         my @nameXmlNodes = $nameNodeSet->get_nodelist();
         foreach my $nameXmlNode (@nameXmlNodes){
            my $textNodeSet = $xpath->find("text()" , $nameXmlNode);
            my $nameText = $textNodeSet->string_value();
            my $lang = getXMLAttributeValue( $xpath, $nameXmlNode, "language" );
            
            my $norecurse=1;
            ${$currCatDef->{'names'}}{$lang}=$nameText;
         }


         push @{$self->{m_categoryDefs}}, $currCatDef;
      }
      foreach my $catDef (@{$self->{m_categoryDefs}}){
         #printCatDef $catDef;
      }
   }
   else {
      errprint "Could not find the \"categories\" node, exits";
      exit 1;
   }

   # 2. Find the tree container node.
   dprint1 "Reading category tree.";
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
         $self->handleXMLCatNode($xpath, 
                                 $node, 
                                 undef);
      }
      
      foreach my $catNode (@{$self->{m_categoryTree}}){
         #printCatNode $catNode, "";
      }
   }
   else {
      errprint "Could not find the \"category_tree\" node, exits";
      exit 1;
   }
}


my %catByTaFoodType = ();
#$catByTaFoodType{0} = ; # Others, not a category, set restaurant.
$catByTaFoodType{1} = 198; # French
$catByTaFoodType{2} = 217; # Belgian
$catByTaFoodType{3} = 208; # Chinese
$catByTaFoodType{4} = 197; # German
$catByTaFoodType{5} = 196; # Greek
$catByTaFoodType{6} = 190; # Italian
$catByTaFoodType{7} = 192; # Indian
$catByTaFoodType{8} = 189; # Japanese
$catByTaFoodType{9} = 178; # Oriental
$catByTaFoodType{10} = 162; # Swiss
$catByTaFoodType{11} = 180; # Mexican
$catByTaFoodType{12} = 159; # Thai
$catByTaFoodType{13} = 204; # Dutch
$catByTaFoodType{14} = 155; # Vietnamese
$catByTaFoodType{15} = 222; # American
$catByTaFoodType{16} = 219; # Austrian
$catByTaFoodType{17} = 214; # British
$catByTaFoodType{18} = 210; # Caribbean
$catByTaFoodType{19} = 223; # African
$catByTaFoodType{20} = 194; # Hawaiian
$catByTaFoodType{21} = 191; # Indonesian
$catByTaFoodType{22} = 187; # Korean
$catByTaFoodType{23} = 201; # Filipino
$catByTaFoodType{24} = 163; # Surinamese
$catByTaFoodType{25} = 193; # Hungarian
$catByTaFoodType{26} = 188; # Jewish
$catByTaFoodType{27} = 174; # Polish
$catByTaFoodType{28} = 171; # Russian
$catByTaFoodType{29} = 158; # Turkish
$catByTaFoodType{30} = 179; # Middle Eastern
$catByTaFoodType{31} = 165; # Spanish
$catByTaFoodType{32} = 173; # Portuguese
$catByTaFoodType{33} = 182; # Maltese
$catByTaFoodType{34} = 212; # Californian
$catByTaFoodType{35} = 185; # Latin American
$catByTaFoodType{36} = 211; # Canadian
$catByTaFoodType{100} = 156; # Vegetarian
$catByTaFoodType{101} = 202; # Fast Food
$catByTaFoodType{102} = 195; # Grill
$catByTaFoodType{103} = 168; # Sea Food
$catByTaFoodType{104} = 170; # Sandwich
$catByTaFoodType{105} = 164; # Steak House
$catByTaFoodType{106} = 216; # Bistro
$catByTaFoodType{107} = 218; # Barbecue
#$catByTaFoodType{255} = ; # Unknown, not a category, set restaurant.

# Static method
# Returns undef if it is not possible to lookup a category of a food type.
sub taFoodTypeToCategory {
   my $foodTypeID = shift;
   return $catByTaFoodType{$foodTypeID};
}



my %catByTaMilServBranch = ();
$catByTaMilServBranch{1} = 145; # Army
$catByTaMilServBranch{2} = 142; # Navy
$catByTaMilServBranch{3} = 146; # Air force
$catByTaMilServBranch{4} = 143; # Marine Corps
$catByTaMilServBranch{5} = 144; # Coast guard

# Static method
# Returns undef if it is not possible to lookup a category of a service branch.
sub taMilitaryServiceBranchToCategory {
   my $milServBranchID = shift;
   return $catByTaMilServBranch{$milServBranchID};
}



my %catByTaTouristAttrType = ();
#$catByTaTouristAttrType{0} = ; # No type
$catByTaTouristAttrType{1} = 131; # Building
$catByTaTouristAttrType{2} = 130; # Monument
$catByTaTouristAttrType{3} = 129; # Natural Attraction
#$catByTaTouristAttrType{4} = ; # Unspecified

#################################
# Service Sub Category
#
my %serviceSubCategory = (

   # Automotive dealer
   # service sub category
   # fallback to feature code
   #'9910001' => '76', # Unspecified
   #'9910002' => '76', # Car
   #'9910003' => '76', # Motorcycle
   #'9910004' => '76', # Boat
   #'9910005' => '76', # Recreational Vehicles
   #'9910006' => '76', # Truck
   #'9910007' => '76', # Van
   #'9910008' => '76', # Bus

   # Car Repair Facility
   # service sub category
   # fallback to feature code
   #'7310001' => '104', # Unspecified
   #'7310002' => '104', # Bodyshops 
   #'7310005' => '104', # Other Repair Shops
   #'7310003' => '104', # Car Glass Replacement Shops
   #'7310006' => '104', # Sale & Installation of Car Accessories
   #'7310004' => '104', # General Car Repair & Servicing
   #'7310007' => '104', # Tyre Services
   #'7310008' => '104', # Motorcycle repair

   # Company 
   # service sub category
   # fallback to feature code
   #'9352001' => '149', # Unspecified
   #'9352002' => '149', # Service
   #'9352003' => '149', # Advertising/Marketing
   #'9352004' => '149', # Computer & Data Services
   #'9352005' => '149', # Computer Software
   #'9352006' => '149', # Diversified Finanacials
   #'9352007' => '149', # Insurance
   #'9352008' => '149', # Mail/Package/Freight Delivery
   #'9352009' => '149', # Real Estate
   #'9352010' => '149', # Savings Institution
   #'9352011' => '149', # Manufacturing
   #'9352012' => '149', # Agricultural Technology
   #'9352013' => '149', # Automobile
   #'9352014' => '149', # Chemicals
   #'9352015' => '149', # Electronics
   #'9352016' => '149', # Mechanical Engineering
   #'9352017' => '149', # Public Health Technologies
   #'9352018' => '149', # Pharmaceuticals
   #'9352019' => '149', # Publishing Technologies
   #'9352020' => '149', # Tele Communications
   #'9352021' => '149', # OEM
   #'9352022' => '149', # Tax Services
   #'9352023' => '149', # Legal Services
   #'9352024' => '149', # Transport
   #'9352025' => '149', # Bus Charter & Rentals
   #'9352026' => '149', # Taxi, Limousine & Shuttle Service
   #'9352027' => '149', # Bus Lines
   #'9352028' => '149', # School Bus
   #'9352029' => '149', # Cleaning Services
   #'9352030' => '149', # Oil & Natural Gas
   #'9352031' => '149', # Mining
   #'9352032' => '149', # Construction
   #'9352033' => '149', # Moving & Storage
   #'9352034' => '149', # Airline
   #'9352035' => '149', # Bridge & Tunnel Operations
   #'9352036' => '149', # Funeral Service & Mortuaries
   #'9352037' => '149', # Investment Advisors
   #'9352038' => '149', # Equipment Rental
   #'9352039' => '149', # Business Services
   #'9352040' => '149', # Cable & Telephone
   #'9352041' => '149', # Automobile Manufacturing

   # Exchange
   # service sub category
   # fallback to feature code
   #'9160001' => '149', # Unspecified
   #'9160002' => '149', # Stock Exchange
   #'9160003' => '149', # Gold Exchange
   #'9160004' => '149', # Currency Exchange XXX this should have it's own cat.
   
   # Hotel/Motel
   # service sub category
   #'7314001' => '', # Unspecified
   '7314002' => '119', # Bed & Breakfast & Guest Houses
   '7314003' => '118', # Hotel
   '7314004' => '117', # Hostel
   '7314005' => '118', # Resort
   #'7314006' => '', # Motel
   #'7314007' => '', # Cabins & Lodges
   
   # Cafe/Pub
   # service sub category
   # fallback to feature code
   #'9376001' => '', # Unspecified
   '9376002' => '228', # Café
   '9376003' => '232', # Pub
   # fallback to feature code
   #'9376004' => '', # Internet Café 
   '9376005' => '225', # Tea House
   #'9376006' => '228', # Coffee Shop
   
   # Restaurant
   # service sub category
   # fallback to feature code
   #'7315001' => '157', # Unspecified
   '7315002' => '223', # African 
   '7315003' => '222', # American 
   '7315004' => '219', # Austrian 
   '7315005' => '218', # Barbecue 
   '7315006' => '217', # Belgian 
   '7315007' => '216', # Bistro 
   '7315008' => '214', # British 
   '7315009' => '212', # Californian 
   '7315010' => '211', # Canadian 
   '7315011' => '210', # Caribbean 
   '7315012' => '208', # Chinese 
   '7315013' => '206', # Crepery 
   '7315014' => '204', # Dutch 
   '7315015' => '202', # Fast Food 
   '7315016' => '201', # Filipino 
   '7315017' => '198', # French 
   '7315018' => '197', # German 
   '7315019' => '196', # Greek 
   '7315020' => '195', # Grill 
   '7315021' => '194', # Hawaiian 
   '7315022' => '193', # Hungarian 
   '7315023' => '192', # Indian 
   '7315024' => '191', # Indonesian 
   '7315025' => '190', # Italian 
   '7315026' => '189', # Japanese 
   '7315027' => '188', # Jewish 
   '7315028' => '187', # Korean 
   '7315029' => '185', # Latin American 
   '7315030' => '157', # Lebanese  # mapped to unknown
   '7315031' => '182', # Maltese 
   '7315032' => '181', # Mediterranean
   '7315033' => '180', # Mexican
   '7315034' => '179', # Middle Eastern
   '7315035' => '178', # Oriental
   '7315036' => '175', # Pizza
   '7315037' => '174', # Polish
   '7315038' => '173', # Portuguese
   '7315039' => '157', # Pub Food # mapped to unknown
   '7315040' => '171', # Russian
   '7315041' => '157', # Roadside # mapped to unknown
   '7315042' => '170', # Sandwich
   '7315043' => '168', # Sea Food
   '7315044' => '165', # Spanish
   '7315045' => '164', # Steak House
   '7315046' => '163', # Surinamese
   '7315047' => '162', # Swiss
   '7315048' => '159', # Thai
   '7315049' => '158', # Turkish
   '7315050' => '156', # Vegetarian
   '7315051' => '155', # Vietnamese
   '7315052' => '208', # Hunan
   '7315053' => '208', # Shandong
   '7315054' => '208', # Guangdong
   '7315055' => '208', # Shanghai
   '7315056' => '208', # Sichuan
   '7315057' => '208', # Dongbei
   '7315058' => '208', # Hot Pot
   '7315059' => '208', # Taiwanese
   '7315060' => '157', # Western & Continental # mapped to unknown
   '7315061' => '157', # Peruvian # mapped to unknown
   '7315062' => '221', # Asian (other)
   '7315063' => '157', # Creole-Cajun # mapped to unknown
   '7315064' => '157', # Soul Food # mapped to unknown (southern USA afroamerican food)
   '7315065' => '157', # Irish # mapped to unknown
   '7315066' => '157', # Jamaican # mapped to unknown
   '7315067' => '186', # Kosher 
   '7315068' => '157', # Czech # mapped to unknown
   '7315069' => '157', # Hamburgers # mapped to unknown
   '7315070' => '157', # Chicken # mapped to unknown
   '7315071' => '157', # Fusion # mapped to unknown
   '7315072' => '185', # Brazilian # mapped to latin american kitchen
   '7315073' => '157', # International # mapped to unknown
   '7315074' => '157', # Moroccan # mapped to unknown
   '7315075' => '157', # Organic # mapped to unknown
   '7315076' => '161', # Tapas 
   '7315077' => '157', # Southwestern # mapped to unknown
   '7315078' => '227', # Ice Cream Parlor
   '7315079' => '157', # Doughnuts # mapped to unknown
   '7315080' => '157', # Slovak # mapped to unknown
   '7315081' => '224', # Afghan
   '7315082' => '157', # Algerian # mapped to unknown
   '7315083' => '157', # Arabian # mapped to unknown
   '7315084' => '185', # Argentinean # mapped to latin american kitchen
   '7315085' => '157', # Armenian # mapped to unknown
   '7315086' => '220', # Australian
   '7315087' => '157', # Basque # mapped to unknown
   '7315088' => '185', # Bolivian # mapped to latin american kitchen
   '7315089' => '157', # Bosnian # mapped to unknown
   '7315090' => '157', # Bulgarian # mapped to unknown
   '7315091' => '157', # Burmese # mapped to unknown
   '7315092' => '157', # Cambodian # mapped to unknown
   '7315093' => '185', # Chilean # mapped to latin american kitchen
   '7315094' => '185', # Colombian # mapped to latin american kitchen
   '7315095' => '157', # Corsican # mapped to unknown
   '7315096' => '157', # Cuban # mapped to unknown
   '7315097' => '157', # Cypriot # mapped to unknown
   '7315098' => '157', # Danish # mapped to unknown
   '7315099' => '210', # Dominican # mapped to carribbean kitchen
   '7315100' => '157', # Egyptian # mapped to unknown
   '7315101' => '214', # English # mapped to british kitchen
   '7315102' => '157', # Ethiopian # mapped to unknown
   '7315103' => '157', # European # mapped to unknown
   '7315104' => '157', # Finnish # mapped to unknown
   '7315105' => '157', # Iranian # mapped to unknown
   '7315106' => '157', # Israeli # mapped to unknown
   '7315107' => '157', # Luxembourgian # mapped to unknown
   '7315108' => '157', # Maghrib # mapped to unknown
   '7315109' => '157', # Mauritian # mapped to unknown
   '7315110' => '157', # Mongolian # mapped to unknown
   '7315111' => '157', # Nepalese # mapped to unknown
   '7315112' => '157', # Norwegian # mapped to unknown
   '7315113' => '157', # Savoyan # mapped to unknown
   '7315114' => '169', # Scandinavian
   '7315115' => '157', # Scottish # mapped to unknown
   '7315116' => '157', # Sicilian # mapped to unknown
   '7315117' => '157', # Slavic # mapped to unknown
   '7315118' => '157', # Sudanese # mapped to unknown
   '7315119' => '157', # Swedish # mapped to unknown
   '7315120' => '157', # Syrian# mapped to unknown
   '7315121' => '157', # Teppanyakki # mapped to unknown (japanese grill)
   '7315122' => '157', # Tibetan # mapped to unknown
   '7315123' => '157', # Tunisian # mapped to unknown
   '7315124' => '185', # Uruguayan # mapped to latin american kitchen
   '7315125' => '185', # Venezuelan # mapped to latin american kitchen
   '7315126' => '214', # Welsh # mapped to british kitchen
   '7315127' => '157', # Pakistani # mapped to unknown
   '7315128' => '157', # Persian # mapped to unknown
   '7315129' => '157', # Polynesian # mapped to unknown
   '7315130' => '157', # Provençal # mapped to unknown
   '7315131' => '157', # Rumanian # mapped to unknown
   '7315132' => '157', # Erotic # mapped to unknown
   '7315133' => '157', # Exotic # mapped to unknown
   '7315134' => '157', # Fondue # mapped to unknown
   '7315135' => '157', # Macrobiotic # mapped to unknown
   '7315136' => '168', # Mussels # mapped to sea food
   '7315137' => '157', # Oven Dishes # mapped to unknown
   '7315138' => '157', # Pasta # mapped to unknown
   '7315139' => '157', # Snacks # mapped to unknown
   '7315140' => '167', # Soup
   '7315141' => '157', # Tex Mex # mapped to unknown
   '7315142' => '213', # Buffet
   '7315143' => '157', # Salad Bar # mapped to unknown
   '7315144' => '157', # Seasonal # mapped to unknown
   '7315145' => '157', # Take away # mapped to unknown XXX add info take away?
   '7315146' => '157', # Banquet Rooms # mapped to unknown
   '7315147' => '228', # Cafeterias # mapped to cafe

   # College / University
   #'7377001' => '254', # Unspecified
   #'7377002' => '254', # College/University
   #'7377003' => '254', # Junior College/Community College
   
   # Government office
   #'7367001' => '112', # Unspecified
   #'7367002' => '112', # Order 9 Area
   #'7367003' => '112', # Order 8 Area
   #'7367004' => '112', # Order 1 Area
   #'7367005' => '112', # National
   #'7367006' => '112', # Supra National

   # Police station
   #'7322001' => '108', # Unspecified
   #'7322002' => '108', # Order 9 Area
   #'7322003' => '108', # Order 8 Area
   #'7322004' => '108', # Order 1 Area

   # School
   #'7372001' => '255', # Unspecified
   #'7372002' => '255', # School
   #'7372003' => '255', # Child Care Facility
   #'7372004' => '255', # Pre School
   #'7372005' => '255', # Primary School
   #'7372006' => '255', # High School
   #'7372007' => '255', # Senior High School
   #'7372008' => '255', # Vocational Training
   #'7372009' => '255', # Technical School
   #'7372010' => '255', # Language School
   #'7372011' => '255', # Sport School
   #'7372012' => '255', # Art School
   #'7372013' => '255', # Special School
   #'7372014' => '255', # Middle School
   #'7372015' => '255', # Culinary School

   # Public amenities
   #'9932001' => '281', # Unspecified
   #'9932002' => '281', # Passenger Transport Ticket Office
   #'9932003' => '281', # Pedestrian Subway
   #'9932004' => '281', # Public Call Box
   #'9932005' => '281', # Toilet
   #'9932006' => '281', # Road Rescue
   
   # Doctor
   #'9373001' => '248', # Unspecified
   #'9373002' => '248', # General Practitioner
   #'9373003' => '248', # Specialist

   # Hospital /Polyclinic
   #'7321001' => '246', # Unspecified
   #'7321002' => '246', # General
   #'7321003' => '246', # Special

   # Health care services
   #'9663001' => '', # Unspecified
   #'9663002' => '', # Personal Services
   #'9663003' => '', # Personal Care Facilities
   
   # Shop
   # service sub category
   #'9361001' => '', # Unspecified
   '9361002' => '77', # Book Shops 
   '9361003' => '63', # CDs, DVD & Videos 
   '9361004' => '75', # Clothing & Accessories: Children 
   '9361005' => '75', # Clothing & Accessories: Footwear & Shoe Repairs
   '9361006' => '75', # Clothing & Accessories: General 
   '9361007' => '75', # Clothing & Accessories: Men 
   '9361008' => '75', # Clothing & Accessories: Women 
   '9361009' => '74', # Convenience Stores 
   '9361010' => '73', # Drycleaners 
   '9361011' => '72', # Electrical, Office & IT: Cameras & Photography
   '9361012' => '72', # Electrical, Office & IT: Computer & Computer Supplies
   '9361013' => '72', # Electrical, Office & IT: Consumer Electronics
   '9361014' => '72', # Electrical, Office & IT: Office Equipment
   '9361015' => '71', # Estate Agents 
   #'9361016' => '', # Factory Outlet 
   '9361017' => '70', # Florists 
   '9361018' => '69', # Food & Drinks: Bakers 
   '9361019' => '69', # Food & Drinks: Butchers 
   '9361020' => '69', # Food & Drinks: Fishmongers 
   '9361021' => '69', # Food & Drinks: Food Markets 
   '9361022' => '69', # Food & Drinks: Green Grocers 
   '9361023' => '67', # Food & Drinks: Grocers # add type 20 | groceryStore
   '9361024' => '69', # Food & Drinks: Other Food Shops
   '9361025' => '69', # Food & Drinks: Wine & Spirits
   '9361026' => '68', # Gifts, Cards, Novelties & Souvenirs
   '9361027' => '66', # Hairdressers & Barbers
   '9361028' => '65', # House & Garden: Carpet/Floor Coverings
   '9361029' => '65', # House & Garden: Curtains/Textiles
   '9361030' => '65', # House & Garden: Do-It-Yourself Centers
   '9361031' => '65', # House & Garden: Furniture & Fittings
   '9361032' => '65', # House & Garden: Garden Centers & Services
   '9361033' => '65', # House & Garden: Kitchens & Bathrooms
   '9361034' => '65', # House & Garden: Lighting
   '9361035' => '65', # House & Garden: Painting & Decorating
   '9361036' => '64', # Jewelry, Clocks & Watches
   '9361037' => '62', # Newsagents & Tobacconists
   '9361038' => '61', # Opticians
   '9361039' => '58', # Sports equipment & clothing
   '9361040' => '56', # Toys & Games
   '9361041' => '55', # Travel Agents
   '9361042' => '65', # Construction Material & Equipment
   '9361043' => '245', # Medical Supplies & Equipment # mapped to pharmacy
   '9361044' => '63', # CD/Video Rental
   '9361045' => '73', # Laundry XXX mapped to dry cleaners
   #'9361046' => '', # Photo Lab/Development
   #'9361047' => '', # Photocopy
   #'9361048' => '', # Animal Services
   '9361049' => '80', # Antique/Art
   #'9361050' => '', # Beauty Supplies
   #'9361051' => '', # Drug Store
   '9361052' => '72', # Electrical Appliance
   #'9361053' => '', # Hobby/Free Time
   '9361054' => '65', # Furniture/Home Furnishings
   #'9361055' => '', # Glassware/Ceramic
   #'9361056' => '', # Local Specialities
   #'9361057' => '', # Recycling Shop
   '9361058' => '75', # Shoes & Bags
   #'9361059' => '', # Musical Instruments
   #'9361060' => '', # Delicatessen
   #'9361061' => '', # Specialty Foods
   #'9361062' => '', # Shopping Service
   #'9361063' => '', # Retail Outlet
   #'9361064' => '', # Pet Supplies
   #'9361065' => '', # Marine Electronic Equipment
   #'9361066' => '', # Wholesale Clubs
   '9361067' => '78', # Beauty Salon
   '9361068' => '78', # Nail Salon
   #'9361069' => '', # Hardware
   
   # Amusement park
   # fallback to feature code
   #'9902001' => '', # Unspecified
   #'9902002' => '', # Amusement Arcade 
   #'9902003' => '', # Amusement Park
   
   # Park & Recreation Area
   # service sub category
   #'9362001' => '43', # Unspecified
   #'9362002' => '43', # Battlefield
   '9362003' => '243', # Cemetery XXX add type Cemetery, 57
   #'9362004' => '43', # Historic Site
   #'9362005' => '43', # Historical Park
   #'9362006' => '43', # Lakeshore
   #'9362007' => '43', # Memorial
   '9362008' => '43', # Park
   #'9362009' => '43', # Parkway
   #'9362010' => '43', # Preserve
   '9362011' => '43', # Recreation Area
   #'9362012' => '43', # River
   #'9362013' => '43', # Seashore
   #'9362014' => '43', # Wilderness Area
   '9362015' => '27', # Forest Area
   #'9362016' => '43', # Fishing & Hunting Area
   #'9362017' => '43', # Fairground
   #'9362018' => '43', # National Park
   #'9362019' => '43', # National Forest
   #'9362020' => '43', # State/Local Park
   #'9362021' => '43', # State Forest
   #'9362022' => '43', # County Forest
   #'9362023' => '43', # Hiking
   #'9362024' => '43', # Horse Riding
   #'9362025' => '43', # Other Winter Sport
   '9362026' => '40', # Ski Resort
   #'9362027' => '43', # Bridge
   #'9362028' => '43', # Mausoleum/Grave
   #'9362029' => '43', # Arch
   '9362030' => '129', # Natural Attraction
   #'9362031' => '43', # Statue
   
   # Theater
   #'7318001' => '', # Unspecified
   '7318002' => '96', # Concert Hall
   '7318003' => '96', # Music Center XXX add type 78 musicCentre
   '7318004' => '89', # Opera XXX add type 79 opera
   '7318005' => '88', # Theater
   '7318006' => '97', # Cabaret
   #'7318007' => '', # Amphitheater
   #'7318008' => '', # Dinner Theater
   
   # Zoo
   # fallback to feature code
   #'9927001' => '', # Unspecified
   #'9927002' => '', # Arboreta & Botanical Gardens
   #'9927003' => '', # Zoo
   #'9927004' => '', # Aquatic Zoo & Marine Park
   #'9927005' => '', # Wildlife Park
   
   # Cinema
   # fallback to feature code
   #'7342001' => '', # Unspecified
   '7342002' => '98', # Cinema 
   #'7342003' => '', # Drive-In Cinema
   
   # Leisure Center
   #'9378001' => '', # Unspecified
   '9378002' => '53', # Bowling XXX add type to 6 bowlingCentre
   #'9378003' => '', # Dance Studio & School
   '9378004' => '42', # Flying Club # mapped to public sport airport
   #'9378005' => '', # Sauna, Solarium & Massage
   #'9378006' => '', # Snooker, Pool & Billiard

   # Nightlife
   #'9379001' => '', # Unspecified
   '9379002' => '94', # Discotheque
   #'9379003' => '', # Private Club
   '9379004' => '100', # Bar
   '9379005' => '235', # Microbrewery/Beer Garden
   '9379006' => '233', # Cocktail Bar
   '9379007' => '229', # Wine Bar
   #'9379008' => '', # Jazz Club
   '9379009' => '97', # Comedy Club
   
   # Sports Center
   #'7320001' => '', # Unspecified
   '7320002' => '37', # Fitness Clubs & Centers
   '7320003' => '51', # Horse Riding
   '7320004' => '37', # Sports Center
   '7320005' => '37', # Thematic Sport
   #'7320006' => '', # Squash Court
   
   # Stadium
   #'7374001' => '', # Unspecified
   #'7374002' => '', # Athletic
   #'7374003' => '', # Cricket Ground
   #'7374004' => '', # Soccer
   '7374005' => '51', # Horse Racing
   #'7374006' => '', # Multi-Purpose
   #'7374007' => '', # Rugby Ground
   #'7374008' => '', # Ice Hockey
   #'7374009' => '', # Baseball
   #'7374010' => '', # Football
   #'7374011' => '', # Motor Sport
   #'7374012' => '', # Basketball
   #'7374013' => '', # Race Track
   
   # Place of Worship
   # fallback to feature code
   #'7339001' => '', # Unspecified
   '7339002' => '242', # Church  add type 96 | church 
   '7339003' => '241', # Mosque  add type  97 | mosque
   '7339004' => '239', # Synagogue  add type  98 | synagogue
   #'7339005' => '', # Temple
   #'7339006' => '', # Gurudwara
   
   # Important tourist attraction
   # fallback to feature code
   #'7376001' => '', # Unspecified
   '7376002' => '131', # Building 
   '7376003' => '130', # Monument 
   '7376004' => '129', # Natural Attraction
   #'7376005' => '', # Observatory
   #'7376006' => '', # Planetarium

   # Airport
   #'7383001' => '', # Unspecified
   #'7383002' => '', # Public Authority
   #'7383003' => '', # Private Authority
   #'7383004' => '', # Military Authority
   #'7383005' => '', # Airfield

   # Public transport stop
   #'9942001' => '', # Unspecified
   '9942002' => '125', # Bus Stop
   '9942003' => '14', # Taxi Stand
   '9942004' => '124', # Tram Stop


); # %serviceSubCategory

#######
   # this is not service sub category, but product category
   # perhaps usable later?
   #
   # Factory Outlet
   #
   # 0 Unspecified 
   # 1 Antiques 
   # 2 Arts 
   # 3 Audio/Video/Photo 
   # 4 Bags & Leather Ware 
   # 5 Beds, Blankets, Mattresses & Accessories
   # 6 Beverages 
   # 7 Bicycles & Accessories 
   # 8 Boats 
   # 9 Cars & Automotive 
   # 10 Carpets 
   # 11 Christmas Articles 
   # 12 Clothes (Children) 
   # 13 Clothes (Men) 
   # 14 Clothes (Women) 
   # 15 Computers & Accessories 
   # 16 Cosmetics 
   # 17 Decorations 
   # 18 Electrical Appliances 
   # 19 Flowers 
   # 20 Foam & Plastics
   # 21 Food
   # 22 Furniture
   # 23 Gifts
   # 24 Glassware, Ceramics & China
   # 25 Haberdashery
   # 26 Household
   # 27 Jewelry & Watches
   # 28 Lights
   # 29 Motorcycles & Accessories
   # 30 Needlework & Craftwork
   # 31 Office Supplies
   # 32 Shoes
   # 33 Sports & Leisure
   # 34 Table Cloth
   # 35 Textiles, Wool & Furs
   # 36 Tools
   # 37 Toys & Childrens Articles
   # 38 Umbrellas
   # 39 Wellness
   
my %serviceSubCategoryToType = (
   #
   '9361023' => '20', # Food & Drinks: Grocers # add type 20 | groceryStore
   '9362003' => '57', # Cemetery # add type Cemetery, 57
   '7318004' => '79', # Opera add type 79 | opera
   '7318003' => '78', # Music Center add type 78 | musicCentre
   '9378002' => '6',  # Bowling # add type to 6 | bowlingCentre
   '7339002' => '96', # Church  add type 96 | church
   '7339003' => '97', # Mosque  add type  97 | mosque
   '7339004' => '98', # Synagogue  add type  98 | synagogue
   '9378004' => '35', # Flying Club mapped to 35 | public sport airport
   '9942002' => '103', # busStop
   '9942003' => '104', # taxi stand => taxi stop
);

sub taServiceSubCategoryToCategory {
   my $serviceSubCategoryValue = shift;
   return $serviceSubCategory{$serviceSubCategoryValue};
}

sub taServiceSubCategoryToType {
   my $serviceSubCategoryValue = shift;
   return $serviceSubCategoryToType{$serviceSubCategoryValue};
}

# Static method
# Returns undef if it is not possible to lookup a category of a tourist attr type
sub taTouristAttrTypeToCategory {
   my $touristAttrTypeID = shift;
   return $catByTaTouristAttrType{$touristAttrTypeID};
}




my %catByTaFerryType = ();
$catByTaFerryType{1} = [128]; # Ferry connection
$catByTaFerryType{2} = [127]; # Rail connection
$catByTaFerryType{3} = [127, 128]; # 

# Static method
# Returns an array of categories. Possibly both reail and ferry. 
# Returns empty array if it is not possible to lookup a category of a service 
# branch.
sub taFerryTypeToCategory {
   my $ferryTypeID = shift;
   return $catByTaFerryType{$ferryTypeID};
}

1;
