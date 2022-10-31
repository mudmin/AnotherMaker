#!/usr/bin/perl

$loader = @ARGV[0];
$asm = @ARGV[1];
$c = @ARGV[2];

open OUT, ">$asm" or die "Can't create $asm: $!\n";
open OUT2, ">$c" or die "Can't create $c: $!\n";
open IN, "<$loader" or die "Can't open $loader: $!\n";
binmode IN;
$/ = \1;  # record size = 1

print OUT "loader_code CODE_PACK 0xF00000\n";
$i = 0;

print OUT2 "static unsigned char loader[] = {\n";

while (<IN>) {
    $b = unpack 'C', $_;

    if ($i % 8 == 0) {
	print OUT "\tdb\t";
	print OUT2 "\t";
    } else {
	print OUT ", ";
	print OUT2 ", ";
    }
    printf OUT "0x%02x", $b;
    printf OUT2 "0x%02x", $b;
    
    $i++;
    if ($i % 8 == 0) {
	print OUT "\n";
	print OUT2 ",\n";
    }
}
print OUT "\n";
print OUT "\tEND\n";

print OUT2 "};\n";

close IN;
close OUT;
close OUT2;

print "$asm generated!\n";


