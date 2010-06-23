#!/usr/bin/perl -w
use File::stat;
use LWP::UserAgent;
use File::Basename;

# This script runs a number of tests

# Some enviorment variables
my $resultDir="results";
# XXX: Try to find java dynamically
#my $javaProg="/usr/java/jdk1.3/bin/java";
my $javaProg="java";
my $imageViewProg="xv";

# XML
my $xmlHost="localhost";
my $xmlPort="12211";
my $xmlLogin="testuser";
my $xmlPasswd="login";

# NS
my $ngpmakerprog = "ngpmaker";
my $nsHost = "localhost";
my $nsPort = "7655";

my @testSet;
my (@xmlTestSet, @nsTestSet);
my @argumentFiles;
my $interactive = 1;
my $noninteractiveretries = 3;

# Check arguments
foreach my $arg (@ARGV) {
    if ( $arg eq "--help" || $arg eq "-h" ) {
	print "Usage: $0 [OPTION] [FILE]\n";
	print "The options are:\n";
	print "                 --noninteractive Just report failed test don't show changes.\n";
	print "If there is any file argument then only tests from those files will be run.\n";
	print "If no file agruments then all tests are run.\n";
	exit 1;
    } elsif ( $arg eq "--noninteractive" ) {
	$interactive=0;
    } elsif ( $arg eq "--no_noninteractiveretries" ) {
	$noninteractiveretries = 1;
    } elsif ( $arg =~ /-.*/ ) { # Starts with - then an option
	print "Unknown option $arg, try -h for help.\n";
	exit 1;
    } else {
	push @argumentFiles, $arg;
    }
}



# Make sure we have a resultDir
my $tmpFile;
if ( ! -d $resultDir) {
    mkdir $resultDir;
}

if ( @argumentFiles == 0 ) {
    # Add XMLServer tests
    push @argumentFiles, './XML/xmltests.pl';
    # Add NavigatorServer tests
    push @argumentFiles, "./NS/nstests.pl";
}


# Read test files
foreach my $testfile (@argumentFiles) {
    open( TESTDATA, $testfile ) || die "Failed to open $testfile\n";
    local $/;                             # set input separator to undef, next stmt will read whole file
    local $testCode = <TESTDATA>;
    # This adds to xmlTestSet or nsTestSet
    eval $testCode;
    close(TESTDATA);
}


push @testSet, @nsTestSet;
push @testSet, @xmlTestSet;

if ( $interactive ) {
    # Print the available commands
    print 
	"When a test fails:\n".
	"Press (a)ccept to accept current reply as valid result.\n".
	"      (d)iff to run tkdiff again.\n".
        "      (e)xit to exit testing and not make any further changes.\n".
        "      (q)uit to quit testing and make no further changes.\n".
        "      (r)un, or\n".
        "      ([enter]) to run test again.\n".
        "      (s)kip to next test, makes no changes.\n";

    while (@testSet) {
 	my ($testName, $test) = @{shift @testSet};
	print "$testName: " . ' ' x (60 - length $testName);
	my $testResult = $testName . "_result";
	my $runTest=1;
	
	while ( $runTest ) {
	    # Run test
	    system( "./$test > $testResult" );
	    my $testStatus = 1;
	    my $hasoldResult = 1;
	    if ( ! -e "$resultDir/$testResult") {
		$hasoldResult = 0;
	    } else {
		$testStatus = system( "diff --brief $resultDir/$testResult $testResult >& /dev/null" );
	    }

	    # Default don't run again
	    $runTest=0;
	    if ( $testStatus != 0 ) {
		local @imageFiles;
		if ( -e $testName . '_images' ) {
		    # display images
		    open( imageUriFile, $testName . '_images' );
		    local $i = 0;
		    while ( <imageUriFile> ) {
			chomp; # Remove newline
			$ua = new LWP::UserAgent;
			# Create a request
			my $req = new HTTP::Request GET => "http://$xmlHost:$xmlPort/$_";
			# Send http request 
			my $res = $ua->request( $req );
			
			# Check the outcome of the response
			if ( $res->is_success ) {
			    open( imageFile, '>', "${testName}_image_$i" );
			    print( imageFile  $res->content );
			    close( imageFile );
			    push @imageFiles, "${testName}_image_$i";
			} else {
			    print "Failed to get image: $_\n";
			}
			$i++;
		    }
		    close( imageUriFile );
		    
		    if ( @imageFiles > 0 ) {
			# Start image viewer in parallell
			$forkRes=fork();
			if ( $forkRes eq 0 ) {
			    system( "$imageViewProg ". join( " ", @imageFiles ) );
			    exit 0;
			}
		    }
		}
		# Promt
		print " Failed: ";

		if ( $hasoldResult ) {
		    system( "tkdiff $resultDir/$testResult $testResult" );
		} else {
		    system( "less $testResult" );
		}
		# Wait for possible image viewer
		wait();
		foreach my $file (@imageFiles) {
		    unlink($file);
		}
		# XXX: Clear stdin
		system "stty cbreak </dev/tty >/dev/tty 2>&1";
		my $ch=getc(STDIN);
		system "stty -cbreak </dev/tty >/dev/tty 2>&1";
		my $handled=0;
		while ( ! $handled ) {
		    while ( $ch !~ m/[easrdq\n]/ ) 
		    {
			system "stty cbreak </dev/tty >/dev/tty 2>&1";
			$ch=getc(STDIN);
			system "stty -cbreak </dev/tty >/dev/tty 2>&1";
		    }
		    # Default is handled
		    $handled=1;
		    # Ok has valid ch -> handle it
		    if ( $ch =~ m/[eq]/ ) {
			print "User abort.\n";
			unlink($testResult);
			exit 0;
		    } elsif ( $ch eq 'a' ) {
			# Set result as new known good result
			if ( $hasoldResult ) {
			    system( "mv -f $resultDir/$testResult $resultDir/${testResult}_`date +%Y%m%d.%H%M`" );
			}
			system( "cp -f $testResult $resultDir/$testResult" );
			print "Result accepted as new result.\n";
		    } elsif ( $ch eq 's' ) {
			# Ok move along
			print "Skipping to next test.\n";
		    } elsif ( $ch eq 'r' ) {
			$runTest = 1;
		    } elsif ( $ch eq 'd' ) {
			system( "tkdiff $resultDir/$testResult $testResult" );
			$handled=0;
			$ch=""; # No valid
		    } elsif ( $ch eq "\n" ) {
			$runTest = 1;
		    }
		}
	    } else {
		print( "OK\n" );
	    }
	}
        
	unlink($testResult);
	if ( -e  $testName . '_images' ) {
	    unlink($testName . '_images' );
	}
    }

} else { # End interactive
    # Non interactive
    $allOk = 1;
    while (@testSet) {
 	my ($testName, $test) = @{shift @testSet};
	print "$testName: " . ' ' x (60 - length $testName);
	my $testResult = $testName . "_result";
	my $runTest=$noninteractiveretries;
	if ( ! -e "$resultDir/$testResult" ) {
	    print( "No old result.\n" );
	    next;
	}

	while ( $runTest ) {
	    # Run test
	    system( "./$test > $testResult" );
	    my $testStatus = 1;
	    $testStatus = system( "diff --brief $resultDir/$testResult $testResult >& /dev/null" );
	    if ( $testStatus != 0 ) {
		# Decrease retry counter
		$runTest--;
		if ( $runTest ) {
		    # Wait before retrying
		    sleep 30;
		} else {
		    # Failed test
		    print "FAILED\n";
		    system( "diff -u $resultDir/$testResult $testResult" );
		    $allOk = 0;
		}
	    } else {
		print "OK\n";
		# Don't run again
		$runTest = 0;
	    }
	}
	unlink($testResult);
    }

    if ( $allOk ) {
	exit 0;
    } else {
	exit 1;
    }
}
