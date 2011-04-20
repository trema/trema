#! /usr/bin/perl

use strict;
use warnings;
use bignum;

&main();

sub main()
{
    while(<STDIN>){
	my $line = $_;
	chomp($line);

	my ($host, $mac);
	if($line =~ /^host\s+(.+?)\s+/){
	    $host = $1;
	}
	elsif($line =~ /hardware ethernet\s+(.+);/){
	    $mac = $1;
	}
	if(defined($host) && defined($mac)){
	    $mac =~ s/://g;
	    my $sql = sprintf("insert into authorized_host (mac,description) values (%s,\"%s\")", hex($mac), $host);
	    system("sqlite3 authorized_host.db \'$sql\'");
	    undef($host);
	    undef($mac);
	}
    }
}
