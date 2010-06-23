#!/usr/bin/perl -w

# return data after first empty line
# reads from std in

my $afterHeader = 0;
while (<>) {
    if ( $afterHeader ) {
	print $_;
    } else {
	chop;
	chop; # Might be CRLF, then this fixes it
	if ( length($_) <= 0 ) {
	    $afterHeader = 1;
	}
    }
}  
