# UE DataTable import notes

Unreal’s CSV DataTable importer expects the **first column** to act as the row name.
If your first column isn’t a unique row key, the import will either fail (duplicate row names)
or silently generate row names you don’t want.

For that reason this export pack includes two variants of most tables:

- **Original generator CSVs** (kept verbatim)
- **UE-friendly CSVs** prefixed with `DT_` and suffixed `_UE.csv` that add a `Name` column first

Use the `_UE.csv` files unless you already have a custom CSV importer.

---

## Dialogue & Decision Suite table design

In `DT_DialogueDecisionSuite_UE.csv`:

- Dialogue rows use Name: `{id}_LINE`
- Decision prompt rows use Name: `{id}_DECISION`
- Option rows use Name: `{id}_OPT_{option_key}`

This keeps the underlying narrative IDs stable (`id`), while giving UE a unique row key per record.

---

## FText vs FString

CSV import to **FText** is doable, but tends to be a pain if you haven’t set up localization.
For early prototyping, import line text as **FString**, and convert to FText when displaying UI.

When you’re ready for localization, migrate the string columns to FText and run them through your loc pipeline.

