#!/usr/bin/perl -w

# This script replaces parameter a with parameter b and 
# parameter c with parameter d and so on.


my @replace;
my @with;
my $isReplace=1;
foreach my $arg (@ARGV) {
    if ( $isReplace ) {
	push @replace, $arg;
    } else {
	push @with, $arg;
    }
    $isReplace = !$isReplace;
}

while ( <STDIN> ) {
    chomp;
    my $foo = $_;
    my $length = @with;
    for ( my $i = 0 ; $i < $length ; $i++ ) {
	local $rep = $replace[ $i ];
	local $wth2 = $with[ $i ];
	$foo =~ s/$rep/$wth2/g;
    }
    print "$foo\n";
}
