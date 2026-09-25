#!/usr/bin/perl
#-----------------------------------------------------------------------------
# translate_dump.pl - put the English content text into initdb/DARKEDEN.sql
#-----------------------------------------------------------------------------
# The seed database came from a Korean/Chinese server: NPC names, zone names,
# monster names, item names, NPC dialogue, system messages and a few dozen
# smaller columns are Korean, Chinese, or Korean bytes read through the wrong
# code page. This script rewrites the content tables of a dump in English and
# leaves player data (accounts, characters, items owned, logs) alone.
#
#	perl tools/i18n/translate_dump.pl <in.sql> <out.sql> [<migration.sql>]
#
# Where the English comes from, in this directory:
#
#	npc_names.en.tsv   every string the dump uses as an NPC name -> English
#	zones.en.tsv       ZoneID -> full name, short name
#	strings.en.tsv     table.column, source text -> English (system
#	                   messages, settings comments, nicknames, last names,
#	                   cash-shop goods, NPC role descriptions, ...)
#	script.en.tsv      NPC dialogue, keyed by the Korean line: the client's
#	                   tools/i18n/npcscript.en.tsv plus the lines only the
#	                   server has (same escaping: \n \r \t \\ \xNN)
#
# and from the dump itself: every *Info table with both a Name and an EName
# column gets Name = EName, MonsterInfo.HName = EName, SkillBalance.HName =
# Name and OptionInfo.HName = Name. Monster names inside MonsterInfo.
# MonsterSummonInfo and the "#name" comments of ZoneInfo's monster lists are
# mapped through MonsterInfo's own HName -> EName pairs, since the server
# looks summoned monsters up by that name (MonsterInfo::getSpriteTypeByName).
#
# The optional migration file holds, for every table the script changed, a
# TRUNCATE and the table's new INSERT, to bring an existing database up to
# date: mysql -u elcastle -D DARKEDEN -p < initdb/migrations/004-english-content.sql
#
# The script fails when a translated value would not fit its column, and
# reports every content cell that is still not ASCII when it is done.
#-----------------------------------------------------------------------------

use strict;
use warnings;
use Encode ();
use FindBin;
use lib $FindBin::Bin;
use SqlDump qw(parse_rows unquote quote join_insert);

my ($inPath, $outPath, $migrationPath) = @ARGV;
die "usage: $0 <in.sql> <out.sql> [<migration.sql>]\n" unless defined $inPath && defined $outPath;
my $dir = $FindBin::Bin;

#-----------------------------------------------------------------------------
# Tables that hold player data or logs: never touched, never reported.
#-----------------------------------------------------------------------------
my %playerData = map { $_ => 1 } qw(
	ARObject BladeObject BugReportLog CoatObject CoupleInfo CoupleRingObject CrashLog CrashReportLog
	CrossObject DeleteChar DonationGuild200501 DonationPersonal200501 EffectBehemothForceScroll
	EffectRestore EnemyErase ExchangeListing FlagSet FlagWarHistory FlagWarStat GoodsList GoodsListObject
	GuildInfo GuildMember GuildVoteDataInfo GuildWarHistory HeadCount ItemTraceLog LarvaObject LogUserInfo
	MaceObject MagazineObject MiniGameScores MofusLog MoneyTraceLog MoonCardObject NicknameBook OpCreate
	Ousters OustersChakramObject OustersCoatObject OustersSkillSave OustersWristletObject PCRoomInfo
	PCRoomLottoObject PetEnchantItemObject Player PotionObject PupaObject QuestItemObject RaceWarPCList
	SMSAddressBook Slang Slayer SpecialEvent SwordObject TrouserObject Vampire VampireAmuletObject
	VampireBraceletObject VampireCoatObject VampireEarringObject VampireRingObject VampireSkillSave
	VampireWeaponObject WarItemObject
);

#-----------------------------------------------------------------------------
# Translation tables
#-----------------------------------------------------------------------------
sub readTsv {
	my ($name, $columns) = @_;
	my $path = "$dir/$name";
	open my $in, '<:raw', $path or die "$path: $!\n";
	my @rows;
	my $n = 0;
	while (my $line = <$in>) {
		$n++;
		$line =~ s/\r?\n\z//;
		next if $line !~ /\t/;    # a line without a tab is a comment
		my @f = split /\t/, $line, $columns;
		die "$path:$n: expected $columns tab separated columns\n" unless @f == $columns;
		push @rows, \@f;
	}
	close $in;
	return @rows;
}

sub unescapeTsv {
	my ($s) = @_;
	$s =~ s/\\(x[0-9a-fA-F]{2}|.)/
		$1 eq 'n' ? "\n" : $1 eq 'r' ? "\r" : $1 eq 't' ? "\t" : $1 eq '\\' ? '\\' :
		$1 =~ m{^x(..)}i ? chr(hex $1) : $1/ge;
	return $s;
}
sub trim { my ($s) = @_; $s =~ s/^\s+//; $s =~ s/\s+\z//; return $s }

my (%npcName, %zone, %string, %script);
for my $r (readTsv('npc_names.en.tsv', 2)) { $npcName{ $r->[0] } = $r->[1] }
for my $r (readTsv('zones.en.tsv', 3)) { $zone{ $r->[0] } = [ $r->[1], $r->[2] ] }
for my $r (readTsv('strings.en.tsv', 3)) { $string{ $r->[0] }{ unescapeTsv($r->[1]) } = unescapeTsv($r->[2]) }
for my $r (readTsv('script.en.tsv', 2)) { $script{ trim(unescapeTsv($r->[0])) } = unescapeTsv($r->[1]) }
for my $t (values %string, \%npcName) {
	for my $v (values %$t) { die "translation is not ASCII: $v\n" if $v =~ /[^\x00-\x7F]/ }
}

#-----------------------------------------------------------------------------
# The dump
#-----------------------------------------------------------------------------
my ($tables, $lines) = parse_rows($inPath);

my (%columns, %width, $current);
for my $line (@$lines) {
	if ($line =~ /^CREATE TABLE `([^`]+)`/) { $current = $1; $columns{$current} = []; next }
	next unless defined $current;
	if ($line =~ /^\s+`([^`]+)`\s+(\w+)(?:\((\d+)\))?/) {
		push @{ $columns{$current} }, $1;
		$width{$current}{$1} = $3 if lc($2) eq 'varchar' || lc($2) eq 'char';
		next;
	}
	undef $current if $line =~ /^\)/;
}
sub col {
	my ($t, $name) = @_;
	my @c = @{ $columns{$t} || [] };
	for (0 .. $#c) { return $_ if $c[$_] eq $name }
	return undef;
}

my (%changed, %widen, @problems);
sub problem { push @problems, join('', @_) }

# Set one cell. A value wider than its column widens the column: the English
# item names are the dump's own EName values, and a few Name columns are
# narrower than the EName column beside them.
sub setCell {
	my ($t, $row, $c, $value) = @_;
	my $name = $columns{$t}[$c];
	my $w = $width{$t}{$name};
	if (defined $w && length($value) > $w) {
		$widen{$t}{$name} = length($value) if ($widen{$t}{$name} // 0) < length($value);
	}
	return if unquote($row->[$c]) eq $value;
	$row->[$c] = quote($value);
	$changed{$t}++;
}

# Translate a cell through a map; a value the map lacks is reported.
sub mapCell {
	my ($t, $row, $c, $map, $what) = @_;
	my $v = unquote($row->[$c]);
	return unless defined $v && $v =~ /[\x80-\xFF]/;
	if (exists $map->{$v}) { setCell($t, $row, $c, $map->{$v}) }
	else { problem("$t.$columns{$t}[$c]: no $what for '$v'") }
}

sub rows { my ($t) = @_; return $tables->{$t} ? @{ $tables->{$t}[1] } : () }

#-----------------------------------------------------------------------------
# Name = EName wherever both exist
#-----------------------------------------------------------------------------
for my $t (sort keys %$tables) {
	next if $playerData{$t};
	my ($n, $e) = (col($t, 'Name'), col($t, 'EName'));
	next unless defined $n && defined $e;
	for my $row (rows($t)) {
		my ($name, $ename) = (unquote($row->[$n]), unquote($row->[$e]));
		next unless defined $name && $name =~ /[\x80-\xFF]/;
		if (defined $ename && $ename =~ /\S/ && $ename !~ /[\x80-\xFF]/) { setCell($t, $row, $n, $ename) }
		else { mapCell($t, $row, $n, $string{"$t.Name"} || {}, 'translation') }
	}
}

#-----------------------------------------------------------------------------
# Monsters: HName = EName, and the names inside summon lists
#-----------------------------------------------------------------------------
my %monsterEN;
{
	my ($h, $e) = (col('MonsterInfo', 'HName'), col('MonsterInfo', 'EName'));
	for my $row (rows('MonsterInfo')) {
		my ($hname, $ename) = (unquote($row->[$h]), unquote($row->[$e]));
		$monsterEN{ trim($hname) } //= $ename if $hname =~ /[\x80-\xFF]/ && $ename =~ /\S/;
	}
	# Chief/boss variants are spelled "치프알칸" for HName "치프 알칸" and so on.
	for my $k (keys %monsterEN) { (my $tight = $k) =~ s/\s+//g; $monsterEN{$tight} //= $monsterEN{$k} }
	# Names the comments use that no MonsterInfo row spells: strings.en.tsv
	# rows under the pseudo column "MonsterName".
	$monsterEN{$_} //= $string{MonsterName}{$_} for keys %{ $string{MonsterName} || {} };
	for my $row (rows('MonsterInfo')) {
		setCell('MonsterInfo', $row, $h, unquote($row->[$e])) if unquote($row->[$h]) =~ /[\x80-\xFF]/;
	}
	my $s = col('MonsterInfo', 'MonsterSummonInfo');
	for my $row (rows('MonsterInfo')) {
		my $v = unquote($row->[$s]);
		next unless defined $v && $v =~ /[\x80-\xFF]/;
		$v =~ s/\(([^(),]*[\x80-\xFF][^(),]*),/exists $monsterEN{trim($1)} ? "(" . $monsterEN{trim($1)} . "," : "($1,"/ge;
		setCell('MonsterInfo', $row, $s, $v);
	}
}

for my $t (qw(SkillBalance OptionInfo)) {
	my ($h, $n) = (col($t, 'HName'), col($t, 'Name'));
	for my $row (rows($t)) {
		setCell($t, $row, $h, unquote($row->[$n])) if unquote($row->[$h]) =~ /[\x80-\xFF]/;
	}
}

#-----------------------------------------------------------------------------
# NPC names wherever they are used
#-----------------------------------------------------------------------------
for my $spec (['NPC', 'Name'], ['Triggers', 'NPC'], ['Triggers_bak', 'NPC'], ['Script', 'OwnerID'],
	['ItemRewardInfo', 'NPC'], ['GatherItemQuestInfo', 'NPC'], ['MonsterKillQuestInfo', 'NPC'],
	['MiniGameQuestInfo', 'NPC'], ['SlayerWeaponRewardInfo', 'NPC'], ['CastleShrineInfo', 'Name']) {
	my ($t, $cn) = @$spec;
	my $c = col($t, $cn);
	next unless defined $c;
	mapCell($t, $_, $c, \%npcName, 'NPC name') for rows($t);
}

#-----------------------------------------------------------------------------
# Zones: names, and the "#name" comments of the monster lists
#-----------------------------------------------------------------------------
for my $t (qw(ZoneInfo ZoneInfo_bak)) {
	my ($id, $full, $short) = (col($t, 'ZoneID'), col($t, 'FullName'), col($t, 'ShortName'));
	for my $row (rows($t)) {
		my $z = $zone{ $row->[$id] };
		for my $pair ([$full, 0], [$short, 1]) {
			my ($c, $i) = @$pair;
			next unless unquote($row->[$c]) =~ /[\x80-\xFF]/;
			if ($z) { setCell($t, $row, $c, $z->[$i]) }
			else { problem("$t: no zone name for ZoneID $row->[$id]") }
		}
	}
	for my $cn (qw(MonsterList EventMonsterList)) {
		my $c = col($t, $cn);
		for my $row (rows($t)) {
			my $v = unquote($row->[$c]);
			next unless defined $v && $v =~ /[\x80-\xFF]/;
			# A comment is "#name" or a bare name before the "(type,count)" group.
			$v =~ s/([^\s(#\x00-\x7F][^\s(#]*)/exists $monsterEN{$1} ? $monsterEN{$1} : $1/ge;
			setCell($t, $row, $c, $v);
		}
	}
}

#-----------------------------------------------------------------------------
# NPC dialogue: the "**" separated segments of Subject and Content
#-----------------------------------------------------------------------------
{
	my ($s, $c) = (col('Script', 'Subject'), col('Script', 'Content'));
	for my $row (rows('Script')) {
		for my $ci ($s, $c) {
			my $v = unquote($row->[$ci]);
			next unless defined $v && $v =~ /[\x80-\xFF]/;
			my @parts = split /(\*\*)/, $v;
			for my $part (@parts) {
				next if $part eq '**' || $part !~ /[\x80-\xFF]/;
				my ($lead, $core, $tail) = $part =~ /^(\s*)(.*?)(\s*)\z/s;
				if (exists $script{$core}) { $part = $lead . $script{$core} . $tail }
				else { problem("Script $row->[0]: no translation for '$core'") }
			}
			setCell('Script', $row, $ci, join('', @parts));
		}
	}
}

#-----------------------------------------------------------------------------
# Everything keyed by table.column in strings.en.tsv
#-----------------------------------------------------------------------------
for my $key (sort keys %string) {
	my ($t, $cn) = split /\./, $key, 2;
	next unless defined $cn && $tables->{$t};
	my $c = col($t, $cn);
	die "strings.en.tsv: no column $key\n" unless defined $c;
	mapCell($t, $_, $c, $string{$key}, 'translation') for rows($t);
}
if ($tables->{GoodsListInfo_bak}) {
	for my $cn (qw(Name Description OptionType)) {
		my $c = col('GoodsListInfo_bak', $cn);
		mapCell('GoodsListInfo_bak', $_, $c, $string{"GoodsListInfo.$cn"} || {}, 'translation') for rows('GoodsListInfo_bak');
	}
}

#-----------------------------------------------------------------------------
# What is still not ASCII in the content tables
#-----------------------------------------------------------------------------
my %left;
for my $t (sort keys %$tables) {
	next if $playerData{$t};
	for my $row (rows($t)) {
		for my $c (0 .. $#$row) {
			next unless $row->[$c] =~ /[\x80-\xFF]/;
			$left{"$t.$columns{$t}[$c]"}++;
		}
	}
}

#-----------------------------------------------------------------------------
# Output
#-----------------------------------------------------------------------------
my @out = @$lines;
my @migration;
for my $t (sort keys %widen) {
	for my $name (sort keys %{ $widen{$t} }) {
		# Round up to the next multiple of ten, at least as wide as EName's 30.
		my $w = $widen{$t}{$name};
		$w = 30 if $w < 30;
		$w = int(($w + 9) / 10) * 10;
		my ($found, $definition);
		for my $i (0 .. $#out) {
			$found = 1 if $out[$i] =~ /^CREATE TABLE `\Q$t\E`/;
			next unless $found;
			if ($out[$i] =~ /^(\s+`\Q$name\E`\s+)(var)?char\(\d+\)(.*?),?\r?\n\z/s) {
				$out[$i] =~ s/(`\Q$name\E`\s+(?:var)?char)\(\d+\)/$1($w)/;
				($definition = $out[$i]) =~ s/^\s+|,?\s*\z//g;
				last;
			}
			last if $out[$i] =~ /^\)/;
		}
		die "no column definition for $t.$name\n" unless defined $definition;
		push @migration, "ALTER TABLE `$t` MODIFY $definition;";
		print STDERR "widened $t.$name to $w (needed $widen{$t}{$name})\n";
	}
}
for my $t (sort keys %changed) {
	my ($prefix, $rows, $first, $last) = @{ $tables->{$t} };
	my $insert = join_insert($prefix, $rows);
	$out[$first] = $insert;
	$out[$_] = '' for $first + 1 .. $last;
	push @migration, "TRUNCATE TABLE `$t`;\n$insert";
}
open my $o, '>:raw', $outPath or die "$outPath: $!\n";
print $o @out;
close $o;

if (defined $migrationPath) {
	open my $m, '>:raw', $migrationPath or die "$migrationPath: $!\n";
	print $m <<'HEADER';
-- English content text. A fresh install gets this from initdb/DARKEDEN.sql;
-- run this once against an existing DARKEDEN database:
--
--   mysql -h 127.0.0.1 -u elcastle -D DARKEDEN -p < initdb/migrations/004-english-content.sql
--
-- Generated by tools/i18n/translate_dump.pl: each content table below is
-- replaced by its English rows. Player data is not touched.
/*!40101 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
HEADER
	print $m "\n", join("\n", @migration);
	print $m "\n/*!40014 SET FOREIGN_KEY_CHECKS=\@OLD_FOREIGN_KEY_CHECKS */;\n";
	close $m;
}

printf STDERR "%d tables rewritten: %s\n", scalar keys %changed, join(', ', map { "$_ ($changed{$_})" } sort keys %changed);
print STDERR "problem: $_\n" for @problems;
printf STDERR "still not ASCII: %s = %d cells\n", $_, $left{$_} for sort keys %left;
exit(@problems ? 1 : 0);
