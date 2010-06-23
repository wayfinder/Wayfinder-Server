<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- Creates the html-skeleton and applies the other templates 
     inside the body-element -->
<xsl:template match="/category_data">
   <html>
   <head><title>Category tree</title></head>
   <body>
      This file was automatically generated from the category tree in xml format.<br/>
      Categories in <i>italic</i> are linked in from other parts of the tree.
      <xsl:apply-templates/>
   </body>
   </html>
</xsl:template>

<xsl:template match="category_tree">
   <h1>Category tree</h1>
   <ul>
      <xsl:apply-templates/>
   </ul>
</xsl:template>

<!-- Handle the links in a special way, expand them
     and make them italic -->
<xsl:template match="cat_node[@cat_link='true']">
   <xsl:variable name="catid" select="@cat_id"/>
   <i>
      <!-- Get the real (non-link) node and expand it here -->
      <xsl:apply-templates select="//cat_node[@cat_id=$catid and not(@cat_link='true')]"/>
   </i>
</xsl:template>

<!-- Handle the regular categories -->
<xsl:template match="cat_node">
   <xsl:variable name="catid" select="@cat_id"/>

   <!-- Make a name that can be linked to, not currently used -->
   <xsl:element name="a">
      <xsl:attribute name="name">
         <xsl:value-of select="$catid"/>
      </xsl:attribute>
   </xsl:element>

   <li>
      <!-- Get the english name of the category from the translations, also show the id -->
      <xsl:value-of select="/category_data/categories/category[@id=$catid]/name[@language='english']"/> (<xsl:value-of select="$catid"/>)
   </li>
   <ul>
      <!-- Put the sub categories in a sub list -->
      <xsl:apply-templates/>
   </ul>
</xsl:template>

<!-- Don't do anything with the categories-node -->
<xsl:template match="categories">
</xsl:template>

</xsl:stylesheet>
