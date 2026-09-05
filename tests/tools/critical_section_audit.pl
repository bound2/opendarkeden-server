#!/usr/bin/env perl
#----------------------------------------------------------------------
# critical_section_audit.pl — report on __ENTER_CRITICAL_SECTION /
# __LEAVE_CRITICAL_SECTION blocks (src/Core/Exception.h).
#
# The block is guarded by a scoped CriticalSection object, so the lock is
# released by the guard on every exit. Two things must therefore never
# appear inside a block:
#
#   * a hand-written x.unlock() on the guarded object — the guard would
#     release a second time when it is destroyed, and unlocking an
#     already unlocked non-recursive pthread mutex is undefined. Use
#     __CRITICAL_SECTION_LOCK.unlock() instead: the guard tracks
#     ownership.
#   * a __LEAVE_CRITICAL_SECTION whose argument differs from its
#     __ENTER's. Harmless now (the argument is unused) but it means the
#     two ends were written for different objects.
#
# Both are reported as ERROR and make the script exit non-zero.
# Everything else — a lock/unlock on some *other* object inside a block,
# an early release through the guard, returns and throws inside a block —
# is reported as INFO only.
#
# Perl, not Python: the dev container ships perl (tests/arch/ already
# uses it) and has no python3.
#
# Usage:
#   perl tests/tools/critical_section_audit.pl            # scans src/
#   perl tests/tools/critical_section_audit.pl FILE...
#   perl tests/tools/critical_section_audit.pl -q         # errors only
#----------------------------------------------------------------------
use strict;
use warnings;
use File::Find;

my $quiet = 0;
my @argv;
for my $a (@ARGV) {
    if ($a eq '-q') { $quiet = 1; next; }
    push @argv, $a;
}

my @files = @argv;
if (!@files) {
    find(
        sub {
            return unless -f $_;
            return unless /\.(?:cpp|h|hpp|inc)$/;
            push @files, $File::Find::name;
        },
        'src'
    );
    @files = sort @files;
}

# The macro definition itself lives here; skip it.
@files = grep { $_ !~ m{(?:^|/)src/Core/Exception\.h$} } @files;

#----------------------------------------------------------------------
# Normalise an __ENTER_CRITICAL_SECTION argument into the receiver text a
# hand-written call on the same object would use:
#
#   m_Mutex                    -> "m_Mutex."          and "m_Mutex->"
#   (*g_pPCFinder)             -> "g_pPCFinder->"     and "g_pPCFinder."
#   (*(pZone->getZoneGroup())) -> "pZone->getZoneGroup()->" / "...()."
#----------------------------------------------------------------------
sub strip_parens {
    my ($e) = @_;
    while ($e =~ /^\((.*)\)$/s) {
        my $inner = $1;
        my $depth = 0;
        my $ok = 1;
        for my $c (split //, $inner) {
            $depth++ if $c eq '(';
            $depth-- if $c eq ')';
            if ($depth < 0) { $ok = 0; last; }
        }
        last unless $ok && $depth == 0;
        $e = $inner;
    }
    return $e;
}

sub receivers {
    my ($arg) = @_;
    $arg =~ s/\s+//g;
    $arg = strip_parens($arg);
    if ($arg =~ /^\*(.*)$/) {
        my $e = strip_parens($1);
        return ("$e->", "$e.");
    }
    return ("$arg.", "$arg->");
}

# Read a file and return its lines with comments blanked out, so a macro or
# a lock() sitting in a comment is not mistaken for code.
sub strip_comments {
    my ($lines) = @_;
    my @out;
    my $inblock = 0;
    for my $l (@$lines) {
        my $c = $l;
        if ($inblock) {
            if ($c =~ s{^.*?\*/}{}) { $inblock = 0; }
            else { $c = ""; }
        }
        if (!$inblock) {
            $c =~ s{/\*.*?\*/}{}g;
            $inblock = 1 if $c =~ s{/\*.*$}{};
            $c =~ s{//.*$}{};
        }
        push @out, $c;
    }
    return @out;
}

my %n = (
    sections  => 0,
    manual    => 0,
    guarded   => 0,
    other     => 0,
    returns   => 0,
    throws    => 0,
    mismatch  => 0,
    unbalanced => 0,
);
my @errors;
my @info;

for my $f (@files) {
    open(my $fh, '<', $f) or die "cannot read $f: $!";
    my @lines = <$fh>;
    close $fh;
    my @code = strip_comments(\@lines);

    my @stack;
    for (my $i = 0; $i < @code; $i++) {
        my $c = $code[$i];
        my $ln = $i + 1;

        if ($c =~ /__ENTER_CRITICAL_SECTION\s*(\(.*)$/s) {
            my $rest = $1;
            my $depth = 0;
            my $arg = "";
            for my $ch (split //, $rest) {
                $depth++ if $ch eq '(';
                $depth-- if $ch eq ')';
                $arg .= $ch;
                last if $depth == 0;
            }
            push @stack, { arg => $arg, line => $ln };
            $n{sections}++;
            next;
        }

        if ($c =~ /__LEAVE_CRITICAL_SECTION\s*(\(.*)$/s) {
            my $rest = $1;
            my $depth = 0;
            my $arg = "";
            for my $ch (split //, $rest) {
                $depth++ if $ch eq '(';
                $depth-- if $ch eq ')';
                $arg .= $ch;
                last if $depth == 0;
            }
            if (!@stack) {
                push @errors, "$f:$ln: __LEAVE_CRITICAL_SECTION without a matching __ENTER";
                $n{unbalanced}++;
                next;
            }
            my $top = pop @stack;
            my ($a, $b) = ($top->{arg}, $arg);
            s/\s+//g for ($a, $b);
            if ($a ne $b) {
                push @errors,
                    "$f:$ln: __LEAVE_CRITICAL_SECTION$b closes __ENTER_CRITICAL_SECTION$a from line $top->{line}";
                $n{mismatch}++;
            }
            next;
        }

        next unless @stack;
        my $top = $stack[-1];
        my $text = $c;
        $text =~ s/^\s+|\s+$//g;

        if ($c =~ /(?:^|[^\w])return\b/)   { $n{returns}++; }
        if ($c =~ /(?:^|[^\w])throw\b/)    { $n{throws}++; }

        next unless $c =~ /\b(?:lock|unlock|trylock)\s*\(\s*\)/;

        if ($c =~ /__CRITICAL_SECTION_LOCK\s*\.\s*(?:lock|unlock)\s*\(\s*\)/) {
            $n{guarded}++;
            push @info, "$f:$ln: guard-mediated release/retake: $text";
            next;
        }

        my $bare = $c;
        $bare =~ s/\s+//g;
        my $guarded_object = 0;
        for my $r (receivers($top->{arg})) {
            if ($bare =~ /\Q$r\E(?:un)?lock\(\);/ || $bare =~ /\Q$r\Etrylock\(\);/) {
                $guarded_object = 1;
                last;
            }
        }

        if ($guarded_object) {
            push @errors,
                "$f:$ln: hand-written lock call on the object guarded by "
              . "__ENTER_CRITICAL_SECTION$top->{arg} (line $top->{line}); "
              . "use __CRITICAL_SECTION_LOCK instead: $text";
            $n{manual}++;
        } else {
            $n{other}++;
            push @info, "$f:$ln: lock call on another object inside a section: $text";
        }
    }

    if (@stack) {
        push @errors, "$f: " . scalar(@stack) . " __ENTER_CRITICAL_SECTION left unclosed (first at line $stack[0]{line})";
        $n{unbalanced}++;
    }
}

if (!$quiet) {
    print "$_\n" for map { "INFO  $_" } @info;
}
print "ERROR $_\n" for @errors;

printf(
    "critical sections: %d   guard-mediated lock calls: %d   other-object lock calls: %d\n",
    $n{sections}, $n{guarded}, $n{other}
);
printf(
    "returns inside sections: %d   throws inside sections: %d\n",
    $n{returns}, $n{throws}
);
printf(
    "ERRORS  hand-written lock calls on the guarded object: %d   ENTER/LEAVE argument mismatches: %d   unbalanced: %d\n",
    $n{manual}, $n{mismatch}, $n{unbalanced}
);

exit($n{manual} + $n{mismatch} + $n{unbalanced} > 0 ? 1 : 0);
