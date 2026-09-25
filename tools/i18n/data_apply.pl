#!/usr/bin/perl
#-----------------------------------------------------------------------------
# data_apply.pl - rewrite the Korean lines of the CP949 data files in English
#-----------------------------------------------------------------------------
# data/SimpleGQuest.xml, data/EventGQuest.xml and the Lua scripts under
# data/lua ship in CP949. This rewrites each file given with every line that
# contains Korean replaced by its English line from the translation table,
# which is keyed by the Korean line (tabs and backslashes escaped as \t and
# \\). The quest files are the same text the client packs in TutorialEtc.rpk,
# so the client's tools/i18n/uitext.en.tsv translates them too.
#
# A file is rewritten only when every Korean line it holds is translated;
# otherwise the missing lines are listed and the file is left alone.
#
#	perl tools/i18n/data_apply.pl <data.en.tsv> <file>...
#-----------------------------------------------------------------------------

use strict;
use warnings;
use Encode ();

my ($enPath, @files) = @ARGV;
die "usage: $0 <data.en.tsv> <file>...\n" unless defined $enPath && @files;

sub unescape {
	my ($s) = @_;
	$s =~ s/\\(.)/$1 eq 't' ? "\t" : $1/ge;
	return $s;
}

my %english;
open my $en, '<:raw:encoding(UTF-8)', $enPath or die "$enPath: $!\n";
my $n = 0;
while (my $line = <$en>) {
	$n++;
	$line =~ s/\r?\n\z//;
	# A line without a tab is a comment (a Korean line may itself start with '#').
	next if $line !~ /\t/;
	my ($korean, $text) = split /\t/, $line, 2;
	die "$enPath:$n: translation is not ASCII\n" if $text =~ /[^\x00-\x7F]/;
	$english{ unescape($korean) } = unescape($text);
}
close $en;

my $failed = 0;
for my $path (@files) {
	open my $in, '<:raw', $path or die "$path: $!\n";
	local $/;
	my $bytes = <$in>;
	close $in;
	my $text = Encode::decode('cp949', $bytes, sub { sprintf '\\x%02X', shift });
	next unless $text =~ /[^\x00-\x7F]/;

	my $eol = $text =~ /\r\n/ ? "\r\n" : "\n";
	my $trailing = $text =~ /\n\z/ ? 1 : 0;
	my @lines = split /\r?\n/, $text, -1;
	pop @lines if $trailing;
	my @missing;
	for my $line (@lines) {
		next unless $line =~ /[^\x00-\x7F]/;
		if (exists $english{$line}) { $line = $english{$line} }
		else { push @missing, $line }
	}
	if (@missing) {
		$failed++;
		print STDERR "$path: ", scalar @missing, " lines untranslated\n";
		print STDERR Encode::encode('UTF-8', "  $_\n") for @missing;
		next;
	}
	my $out = join($eol, @lines) . ($trailing ? $eol : '');
	open my $o, '>:raw', $path or die "$path: $!\n";
	print $o $out;
	close $o;
	print STDERR "$path: rewritten\n";
}
exit($failed ? 1 : 0);
