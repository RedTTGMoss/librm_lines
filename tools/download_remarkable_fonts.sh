#/bin/sh

# Links found in /usr/share/remarkable/webui/assets/index.js from a Remarkable Paper
# Pro, firmware version v3.24.0.149.
# Additionally that directly also contains reMarkableSerif.woff2 and reMarkableSans.woff2

set -e
(
cd rm_lines/assets/fonts
curl https://cdn.sanity.io/files/xpujt61d/production/47ed70b8382b19b3487648982b78a7b2ada3eb3f.woff2 --output reMarkableSerifItalic.woff2
curl https://cdn.sanity.io/files/xpujt61d/production/f75f89732cba9023fa578d7fd28666798de505d0.woff2 --output reMarkableSerif.woff2
curl https://cdn.sanity.io/files/xpujt61d/production/227f58180ac8527c16669879375e917f9d5ab6e4.woff2 --output reMarkableSans.woff2

# Small conversion from woff2 to ttf
woff2_decompress reMarkableSerifItalic.woff2
woff2_decompress reMarkableSerif.woff2
woff2_decompress reMarkableSans.woff2

rm reMarkableSerifItalic.woff2
rm reMarkableSerif.woff2
rm reMarkableSans.woff2
)