# UI font pipeline

IrOS can build its UI bitmap atlas from the official Vazirmatn Regular TrueType font at build time.
The build downloads `Vazirmatn-Regular.ttf` from the upstream `rastikerdar/vazirmatn` project and converts the printable ASCII glyphs into a compact kernel bitmap atlas with `tools/fontpack.py`.
The TTF file itself is not copied into the final IrOS ISO or disk image.

Vazirmatn is distributed under the SIL Open Font License 1.1. See the upstream project for the complete license and authorship information.
