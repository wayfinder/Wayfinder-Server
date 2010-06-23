#!/usr/bin/perl -w

# Finds the uris in xml reply input
# Extracts all between start and end

my @start;
my @end;


push @start, "link><![CDATA[";
push @end, "]]></";

push @start, "href=\"";
push @end, "\"";

push @start, "<href><![CDATA[";
push @end, "]]></href>";

while ( <STDIN> ) {
    chomp;
    my $foo = $_;
    my $length = @start;
    for ( my $i = 0 ; $i < $length ; $i++ ) {
	local $st = $start[ $i ];
	$startPos = index($foo,$st);
	if ( $startPos ne -1 ) {
	    # Found start
	    $startPos += length($st);
	    local $en = $end[ $i ];
	    $endPos = index($foo,$en,$startPos);
	    if ( $endPos != -1 ) {
		system( "echo '". substr( $foo, $startPos, ($endPos-$startPos ) ). "'" . "| `dirname $0`/replace.pl '&amp;' '&'" );
	    }
	}
    }
}
