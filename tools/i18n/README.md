# English content text

The seed database (`initdb/DARKEDEN.sql`), the quest lists (`data/SimpleGQuest.xml`,
`data/EventGQuest.xml`) and the Lua event scripts came from a Korean server by
way of a Chinese one, so their text was Korean, Chinese, or Korean bytes read
through the wrong code page. The English lives in the tables here and is
written into those files by two scripts. The client repository holds the
matching translations for the data it ships (`tools/i18n` there); the NPC,
zone, monster and item spellings are shared, so a name changes in both places.

| File | What | Used by |
|---|---|---|
| `npc_names.en.tsv` | every string the dump uses as an NPC name -> English (the client's `npc.en.tsv` spellings) | `translate_dump.pl` |
| `zones.en.tsv` | ZoneID -> full name, short name | `translate_dump.pl` |
| `strings.en.tsv` | table.column, source text -> English: system messages (`GSStringPool`, `SSStringPool`), setting comments, master speech, nicknames, monster last names, cash-shop goods, NPC role descriptions, and `MonsterName` rows for monster-list comments no `MonsterInfo` row spells | `translate_dump.pl` |
| `script.en.tsv` | NPC dialogue keyed by the Korean line: the client's `npcscript.en.tsv` plus the lines only the `Script` table has | `translate_dump.pl` |
| `data.en.tsv` | the Korean lines of the quest XML and Lua files -> English (the quest rows are the client's `uitext.en.tsv` rows for the same files) | `data_apply.pl` |

## Regenerating the dump

```bash
perl tools/i18n/translate_dump.pl initdb/DARKEDEN.sql initdb/DARKEDEN.sql initdb/migrations/004-english-content.sql
perl tools/i18n/data_apply.pl tools/i18n/data.en.tsv data/SimpleGQuest.xml data/EventGQuest.xml data/lua/*.lua data/lua/*/*.lua
```

`translate_dump.pl` rewrites the content tables and leaves player data alone
(its `%playerData` list). Besides the tables above it applies the dump's own
English: every `*Info` table with both `Name` and `EName` gets `Name =
EName`, `MonsterInfo.HName = EName`, `SkillBalance.HName = Name` and
`OptionInfo.HName = Name`; monster names inside `MonsterInfo.MonsterSummonInfo`
and the `#name` comments of the zone monster lists go through the same
`HName -> EName` pairs, because the server looks summoned monsters up by that
name (`MonsterInfo::getSpriteTypeByName`). A translated value wider than its
column widens the column (three did: `DyePotionInfo.Name`,
`EventQuestRewardInfo.Name`, `NicknameIndex.Nickname`). The script fails on a
name it has no translation for and ends by listing every content cell that is
still not ASCII, which should be none.

The migration file it writes replaces each rewritten table's rows in an
existing database; see `initdb/migrations/004-english-content.sql`.

Both scripts need only Perl (with `Encode`, in the core distribution).
`SqlDump.pm` beside them parses and rewrites the dump's extended INSERTs.
