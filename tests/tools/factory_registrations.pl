#!/usr/bin/perl
# Prints the packet factories each server registers, one "server<TAB>Factory"
# line per registration, sorted -- derived from the FactoryList type lists
# and the per-server Concat selection in src/Core/PacketFactoryManager.cpp.
#
# tests/ratchet/ratchets.sh diffs this against the committed
# tests/ratchet/factory_registrations.txt, so a registration that is added
# to OR dropped from any server's set fails the ratchet. The inventory check
# next to it only proves registered <= inventory; it cannot see a dropped
# registration (that is how CLRegisterPlayerFactory went missing from the
# loginserver once, caught only in review). Regenerate the committed file
# with
#     perl tests/tools/factory_registrations.pl > tests/ratchet/factory_registrations.txt
# when a membership change is intended, and say so in the commit.
#
# Usage: perl tests/tools/factory_registrations.pl [PacketFactoryManager.cpp]
use strict;
use warnings;

my $file = $ARGV[0] // 'src/Core/PacketFactoryManager.cpp';
open my $fh, '<', $file or die "$file: $!";
local $/;
my $src = <$fh>;
close $fh;
$src =~ s/\r//g;

# using <Name>Factories = FactoryList<  A,  B, ... >;
my %lists;
while ($src =~ /^using (\w+) = FactoryList<\n(.*?)>;/msg) {
    my ($name, $body) = ($1, $2);
    $lists{$name} = [$body =~ /(\w+Factory)\b/g];
}
die "no FactoryList type lists found in $file\n" unless %lists;

# The per-server selection: one `using ServerFactories = ...;` under each
# server macro's #if/#elif arm. The order of the arms is not assumed.
my %selection;
while ($src =~ /^#(?:el)?if defined\((__(?:GAME|LOGIN|SHARED)_SERVER__)\)\nusing ServerFactories = (.*?);/msg) {
    $selection{$1} = $2;
}
for my $macro (qw(__GAME_SERVER__ __LOGIN_SERVER__ __SHARED_SERVER__)) {
    die "no ServerFactories selection for $macro in $file\n" unless $selection{$macro};
}

my %server = (
    __GAME_SERVER__   => 'gameserver',
    __LOGIN_SERVER__  => 'loginserver',
    __SHARED_SERVER__ => 'sharedserver',
);
my @out;
for my $macro (sort keys %server) {
    my @names;
    for my $list ($selection{$macro} =~ /(\w+Factories)\b/g) {
        die "$macro selects unknown list $list\n" unless $lists{$list};
        push @names, @{$lists{$list}};
    }
    my %seen;
    for my $n (@names) {
        die "$server{$macro} registers $n twice\n" if $seen{$n}++;
        push @out, "$server{$macro}\t$n\n";
    }
}
print sort @out;
