#!/usr/bin/perl

#
# Copyright (C) 2013 Frederic Vecoven
#
# This file is part of trs_hard
#
# trs_hard is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# trs_hard is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#


use strict;
use Digest::CRC;
use File::Copy;

my $project_dir = $ARGV[0];

my $file = $project_dir . "FatFS.hex";
my $boot_inc = $project_dir . "bootloader.inc";
my $version_h = $project_dir . "version.h";
my $crc_file = "crc.inc";
my $file_out = $project_dir . $crc_file;
my $F;

if (! -f $file) {
    # FatFS.hex doesn't exist yet
    make_inc($file_out, 0);
    exit 0;
}

# read previous crc
my $old_crc = 0;
if (-f $file_out) {
    open($F, "< $file_out") or die "Can't open $file_out : $!\n";
    while (<$F>) {
	if (/^THECRC\s+equ\s+([0-9A-Fa-fx]+)/) {
	    $old_crc = hex($1);
	}
    }
    close($F);
}


# parse hex file
my $linenum = 0;
my @data;
my $addr = 0;
my $i;

for ($i = 0; $i < 0x10000; $i++) {
    $data[$i] = 0xff;
}

open($F, "< $file") or die "Can't open $file : $!\n";
while (<$F>) {
    $linenum++;
    s/\r$//;
    chomp;
    if (/^:/) {
	my $line = $_;
	my $binrec = pack('H*', substr($_, 1));
	my $check = unpack("%8C*", $binrec);
	die "Invalid checksum at line $linenum" unless $check == 0;

	my ($addr2, $type, $data2) = unpack("x n C X4 C x3 /a", $binrec);

	if ($type == 4) {
	    # upper address
	    $addr &= 0x0000ffff;
	    $addr |= ($addr2 << 16);
	} elsif ($type == 0) {
	    $addr &= 0xffff0000;
	    $addr |= $addr2;
	    if ($addr < 0x10000) {
		$i = 0;
		map( $data[$addr + $i++] = hex($_),
		     unpack('H2' x length($data2), $data2));
	    }
	}
    }
}
close($F);

my $digest = Digest::CRC->new(type => "crc16");
for ($i = 0x800; $i < 0xFFFE; $i++) {
    $digest->add(chr $data[$i]);
}
my $new_crc = $digest->digest;

make_inc($file_out, $new_crc);

# check bootloader file for PROTEUS
my $proteus = 0;
open ($F, "<$boot_inc") or die "Can't open $boot_inc : $!\n";
while (<$F>) {
    if (/^\#define\s+PROTEUS_SIMULATOR\s+([0-9]+)/) {
	if ($1 != 0) {
	    print "***********************************\n";
	    print "**** PROTEUS SIMULATOR VERSION ****\n";
	    print "***********************************\n";
	    $proteus = 1;
	}
	last;
    }
}
close($F);

# warn the user to recompile if the crc is new
if ($old_crc != $new_crc) {
    print "\n";
    print "*** New CRC detected. Please recompile ***\n";
    print "\n";
    exit 0;
}

# get version major and minor
my $version_major = -1;
my $version_minor = -1;
open ($F, "<$version_h") or die "Can't open $version_h : $!\n";
while (<$F>) {
    if (/^\#define\s+VERSION_MAJOR\s+([0-9]+)/) {
	$version_major = $1;
    } elsif (/^\#define\s+VERSION_MINOR\s+([0-9]+)/) {
	$version_minor = $1;
    }
}     
close($F);

if ($version_major == -1 || $version_minor == -1) {
    print "Can't parse version info from $version_h\n";
} else {
    my $frehd = sprintf "FreHD_%d.%02d.hex", $version_major, $version_minor;
    my $newhex = $project_dir . $frehd;
    # don't copy firmware if proteus is set
    if ($proteus == 0) {
	copy($file, $newhex);
	my $str = "**** Generated " . $frehd . " ***";
	print "*" x length($str), "\n";
	print $str, "\n";
	print "*" x length($str), "\n";
    }
}


exit 0;


sub make_inc {
    my $file = shift;
    my $crc = shift;
    
    open(my $OUT, " > $file") or die "Can't create $file : $!\n";
    printf $OUT "THECRC\tequ\t0x%04x\n", $crc;
    close($OUT);
    
    # print "Generated $crc_file\n";
}
