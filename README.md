# bigram_quoter
An artificial intelligence that generates fake quotes using a bigram model and example text.

## Commands
Command-line interactions consist of stashing new or loaded bigram quoters and applying operations to them. This may be done multiple times in any order. But a quoter must be stashed before operations will do anything. Stashed bigram quoters will remain in the stash until all given arguments have been parsed. After all arguments have been parsed, all stashed bigram quoters will be written to their respective save files.
* **-s, --strict**
  Switch to strict mode for all proceeding commands. Exit on non-fatal errors.

* **-t, --tolerant**
  Switch to tolerant mode for all proceeding commands. Only exit on fatal commands. Enabled by default.

* **-n, --new [FILES]**
  Creates a new bigram quoter and adds it to the stash.

* **-o, --overwrite [FILE]**
  Overwrites an existing bigram quoter with a new bigram quoter.

* **-l, --load [FILE]**
  Loads data for a bigram quoter from save files and adds them to the stash.

* **-m, --merge [FILE]**
  Merge stashed bigram quoters into a new bigram quoter. The new bigram quoters will be added to the stash. The stashed bigram quoters that are merged will not be manipulated.

* **-f, --feed [FILE]**
  Feeds a text file into the stashed bigram quoters.

* **-b, --build**
  Constructs a single sentences for each stashed bigram quoter.
