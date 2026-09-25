package SqlDump;
# Shared helpers for reading and writing the extended INSERT rows of a
# mysqldump file. Values are kept as the SQL tokens the dump spells them
# with ('quoted string' with SQL escapes, bare numbers, NULL), so a row that
# is not translated is written back with the same bytes (blanks between the
# tokens, which a few hand-edited tables carry, are the one thing dropped).
use strict;
use warnings;
use Exporter 'import';
our @EXPORT_OK = qw(parse_rows unquote quote split_insert join_insert);

sub _skip_blank {
    my ($body, $pos) = @_;
    $pos++ while $pos < length($$body) && substr($$body, $pos, 1) =~ /\s/;
    return $pos;
}

# Splits "INSERT INTO `T` VALUES (..),(..);" into its rows: returns the
# "INSERT INTO `T` VALUES " prefix and [ [tok, tok, ...], ... ].
sub split_insert {
    my ($line) = @_;
    $line =~ s/\r?\n\z//;
    my ($prefix, $body) = $line =~ /^(INSERT INTO `[^`]+` VALUES )(.*);\s*\z/s
        or die "not an INSERT line";
    my @rows;
    my $pos = 0;
    my $len = length $body;
    while ($pos < $len) {
        $pos = _skip_blank(\$body, $pos);
        last if $pos >= $len;
        die "expected '(' at $pos" unless substr($body, $pos, 1) eq '(';
        $pos++;
        my @row;
        for (;;) {
            $pos = _skip_blank(\$body, $pos);
            my $c = substr($body, $pos, 1);
            if ($c eq "'") {
                my $start = $pos;
                $pos++;
                while ($pos < $len) {
                    my $d = substr($body, $pos, 1);
                    if ($d eq '\\') { $pos += 2; next }
                    if ($d eq "'") { $pos++; last }
                    $pos++;
                }
                push @row, substr($body, $start, $pos - $start);
            }
            else {
                my $start = $pos;
                $pos++ while $pos < $len && substr($body, $pos, 1) !~ /[,)]/;
                (my $tok = substr($body, $start, $pos - $start)) =~ s/\s+\z//;
                push @row, $tok;
            }
            $pos = _skip_blank(\$body, $pos);
            my $sep = substr($body, $pos, 1);
            $pos++;
            last if $sep eq ')';
            die "expected ',' or ')' at $pos" unless $sep eq ',';
        }
        push @rows, \@row;
        $pos = _skip_blank(\$body, $pos);
        $pos++ if substr($body, $pos, 1) eq ',';
    }
    return ($prefix, \@rows);
}

# The inverse: one INSERT line (with its newline) from prefix and rows.
sub join_insert {
    my ($prefix, $rows) = @_;
    return $prefix . join(',', map { '(' . join(',', @$_) . ')' } @$rows) . ";\n";
}

# 'quoted' SQL token -> raw bytes (undef for NULL, bare tokens unchanged).
sub unquote {
    my ($tok) = @_;
    return undef if $tok eq 'NULL';
    return $tok unless $tok =~ /^'(.*)'\z/s;
    my $s = $1;
    $s =~ s/\\(.)/$1 eq 'n' ? "\n" : $1 eq 'r' ? "\r" : $1 eq 't' ? "\t" : $1 eq '0' ? "\0" : $1 eq 'Z' ? "\x1A" : $1/gse;
    return $s;
}

# raw bytes -> 'quoted' SQL token with mysqldump's escaping.
sub quote {
    my ($s) = @_;
    return 'NULL' unless defined $s;
    $s =~ s/\\/\\\\/g;
    $s =~ s/'/\\'/g;
    $s =~ s/\n/\\n/g;
    $s =~ s/\r/\\r/g;
    $s =~ s/\t/\\t/g;
    $s =~ s/\0/\\0/g;
    $s =~ s/\x1A/\\Z/g;
    return "'$s'";
}

# Reads the dump and returns ({ table => [prefix, rows, firstLine, lastLine] },
# \@lines) for the tables named (all tables when the list is empty). An
# INSERT that runs over several lines of the file is joined before parsing.
sub parse_rows {
    my ($path, @tables) = @_;
    my %want = map { $_ => 1 } @tables;
    open my $in, '<:raw', $path or die "$path: $!";
    my @lines = <$in>;
    close $in;
    my %tables;
    my $i = 0;
    while ($i <= $#lines) {
        if ($lines[$i] =~ /^INSERT INTO `([^`]+)` VALUES /) {
            my $t = $1;
            my $start = $i;
            my $text = $lines[$i];
            while ($text !~ /\);\s*\z/ && $i < $#lines) {
                $i++;
                $text .= $lines[$i];
            }
            if (!@tables || $want{$t}) {
                my ($prefix, $rows) = split_insert($text);
                $tables{$t} = [$prefix, $rows, $start, $i];
            }
        }
        $i++;
    }
    return (\%tables, \@lines);
}

1;
