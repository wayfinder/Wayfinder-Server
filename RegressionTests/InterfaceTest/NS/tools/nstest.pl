#!/usr/bin/perl -w
use Socket;
use strict;

my $testFile = '';
my $http = 1;
my $host = "testhost";
my $port = 80;
my $outFile = '';

# Check arguments
foreach my $arg (@ARGV) {

    if ( $arg eq "--help" || $arg eq "-h" ) {
	# You must be kidding!? Right?
	print "Usage:   $0 [options] testfile\n";
	print "Options: -h                  \n".
	      "         --help              This help.\n".
	      "         --navprot           Use raw navprot, default is http.\n".
	      "         --file=[filename]   Optional file to save reply data in.\n".
	      "         --host=[hostname]   Set the host to connect to.\n".
	      "         --port=[portnumber] Set the port to connect to.\n";
	exit 1;
    } elsif ( $arg eq "--navprot" ) {
	$http = 0;
   } elsif ( $arg =~ /--file=.*/ ) {
	$outFile = substr( $arg, 7 );
    } elsif ( $arg =~ /--host=.*/ ) {
	$host = substr( $arg, 7 );
    } elsif ( $arg =~ /--port=.*/ ) {
	$port = substr( $arg, 7 );
    } elsif ( $arg =~ /^-.*/ ) { # Starts with - then an option
        print "Unknown option $arg, try --help for help.\n";
        exit 1;
    } else {
        $testFile = $arg;
    }
}

# Get test file
my $file;
if ( ! open( $file, $testFile ) ) {
    print "Failed to open file \"$testFile\"\n";
    exit 1;
}

my $data = '';
my $t;
my $size;
($t,$t,$t,$t,$t,$t,$t, $size, $t,$t,$t,$t,$t) = stat( $testFile );
my $dataSize = $size;
if ( read( $file, $data, $dataSize ) != $dataSize ) {
    print "Failed to read all of file ".$testFile."\n";
    exit 1;
}
close( $file );


# Socket operations
my $res = 1;
my $sock;
my $result;
my $content = '';
my $status = "OK";


if ( $res && !(socket( $sock, AF_INET, SOCK_STREAM, getprotobyname('tcp') ), $sock) ) 
{
    $status = "socket_create() failed.";
    $res = 0;
}

if ( $res ) {
    my $iaddr = inet_aton( $host );
    if ( $iaddr ) {
	my $paddr = sockaddr_in( $port, $iaddr );
	if ( !($result = connect( $sock, $paddr ) ) ) {
	    $status = "socket_connect() failed.";
	    $res = 0;
	}
    } else {
	$status = "Failed to lookup host " . $host;
	$res = 0;
    }
}


if ( $res && $http ) {
    # Http Header
    my $httpHeader = "POST /nav HTTP/1.1\r\nContent-Length: ".$dataSize.
	"\r\nHost: a\r\n\r\n";
    
    my $hsize = length( $httpHeader );
    if ( ($result = syswrite( $sock, $httpHeader, $hsize )) != $hsize ) {
	$status = "socket_write failed to write http header.";
	$res = 0;
    }
}

if ( $res && ($result = syswrite( $sock, $data, $dataSize )) != $dataSize )
{
    $status = "socket_write failed to write all data.";
    $res = 0;
}

if ( $res ) {
    shutdown( $sock, 1 );
}


my $hasContent = 0;
my $out;
while ( $res && sysread( $sock, $out, 4096 ) ) {
    $content .= $out;
    $hasContent = 1;
    $out = '';
}


if ( $sock ) {
    close( $sock );
}


if ( $res && !$hasContent ) {
    $status = "Empty answer.";
} elsif ( $res && $http ) {
    # Strip http header
    $content =~ s/.*\r\n\r\n//s;
}

# Write to outFile
if ( $outFile ) {
    my $f;
    if ( ! open( $f, ">", $outFile ) ) {
	print "Failed to open file \"$outFile\" to save reply in.\n";
    }
    my $outLength = length( $content );
    if ( syswrite( $f, $content, $outLength ) != $outLength ) {
	print "Failed to write all of reply to: ".$outFile."\n";
    }
    close( $f );
}


if ( $hasContent && ($status =~ /^OK$/) ) {
    if ( ord( substr( $content, 10, 1 ) ) != 0x37 ) {
	$status = "Reply status is not OK.";
    }
} else {
    if ( $status =~ /^OK$/ ) {
	$status = "No content!";
    }
}

print $status . "\n";

if (  $status =~ /^OK$/ ) {
   exit 0;
} else {
   exit 1;
}

