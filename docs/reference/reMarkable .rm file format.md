reMarkable tablets let you draw, annotate, highlight and write in and on top of documents.
All this data is stored in `.rm` files.

This reference concerns the 6th version of the reMarkable file version, see [the versions section](#versions).

## .rm file Versions

Each .rm file starts with a version header, it is always exactly 43 bytes long.

The header follow this exact format: `reMarkable .lines file, version={number}`, and it is padded with 0s until 43
bytes.

The following versions have appeared in files created by reMarkable at different software releases.

- `"reMarkable lines with selections and layers"` - There was no explicit version number in the earliest reMarkable
  software
- `"reMarkable .lines file, version=3          "` - This is the first known versioned file format
- `"reMarkable .lines file, version=5          "`
- `"reMarkable .lines file, version=6          "` - This is the current version

Note that versions 1, 2 and 4 have never appeared in public builds. These might have been internal builds, or were
skipped over for another reason.

First, the file stores all data using [CRDTs](https://en.wikipedia.org/wiki/Conflict-free_replicated_data_type), this is
done so that the same file can be written to by multiple devices in a deterministic manner. For more info on the CRDTs,
read [the CRDTs section below](#crdts)

## Scene tree

## Blocks

## CRDTs

## Sources

1. Early file versions
   history: [reMarkable .lines File Format on Plasma ninja's blog](https://plasma.ninja/blog/devices/remarkable/binary/format/2017/12/26/reMarkable-lines-file-format.html)
