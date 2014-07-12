#!/usr/bin/perl
# Wandeln aller Bilder in eine bestimmte Groesse
#

use strict;

my $ext_list = ";jpg;png;tiff;";
my $s_verbose = 1;

sub Help {
	print <<EOS;
Aufruf: conv_img.pl src_dir target_dir max_width max_height
Bsp:
conv_img.pl . /tmp/new 1024 768
+++ $_[0]
EOS
	exit 1;
}

my $dir_src = shift;
my $dir_trg = shift;
my $max_width = shift;
my $max_height = shift;

&Help ("zu wenig Parameter") unless $max_height;
&Help ("seltsame Breite: $max_width") unless $max_width > 10;
&Help ("seltsame Hoehe: $max_height") unless $max_height > 10;
&Help ("kein Quell-Verzeichnis: $dir_src") unless -d $dir_src;
&Help ("kein Ziel-Verzeichnis: $dir_trg") unless -d $dir_trg;

opendir (DIR, $dir_src) || die "$dir_src: $!";
my @files = readdir (DIR);
close (DIR);

my ($fn_src, $fn_trg, $info, $w, $h, $w2, $h2, $cmd);

foreach (@files) {
	next if /^.{1,2}$/ || -d;
	/^(.*)\.([^.]+)$/;
	my ($fn, $ext) = ($1, $2);
	$ext =~ tr/A-Z/a-z/;
	next if $ext_list !~ /;$ext;/;
	$fn_src = $dir_src . "/" . $_;
	&Convert ($fn_src, "$dir_trg/$fn.$ext", $max_width, $max_height);
}
exit 0;

sub NewExt{
	my $fn = shift;
	my $ext = shift;
	$fn =~/^(.*)\.([^.]+)$/;
	return $1 . "." . $ext;
} # NewExt

sub Convert {
	my $fn_src = shift;
	my $fn_trg = shift;
	my $max_width = shift;
	my $max_height = shift;

	my ($w, $h, $w2, $h2, $cmd, $factor);

	my $info = `identify $fn_src`;
	if ($info !~ / (\w+) (\d+)x(\d+)/) {
		print "+++ kann Format nicht erkennen: $info";
	} elsif ($2 == 0) {
		print "+++ H�he 0: $info\n";
	} else {
		($w, $h) = ($2, $3);
		if ($w / $h > $max_width / $max_height){
			$factor = $max_width / $w;
		} else {
			$factor = $max_height / $h;
		}
			
		if ($w <= $max_width && $h <= $max_height) {
			$cmd = "cp '$fn_src' '$fn_trg'";
		} else {
			$h2 = int ($factor * $h);
			$w2 = int ($factor * $w);
			$cmd = "convert -size ${w}x$h '$fn_src' -resize ${w2}x$h2 '$fn_trg'";
		} # conversion is needed
		print $cmd, "\n" if $s_verbose;
		system ($cmd);
	} # ! ($info =~ //)
} # Convert
