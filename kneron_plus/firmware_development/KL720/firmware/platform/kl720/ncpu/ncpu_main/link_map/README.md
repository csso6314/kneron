# Note fot the build_ldscripts.bat
***
1. User MUST execute build_ldscripts.bat if the memmap.xmm is changed, otherwise the memory allocation will be the same.
2. Change the path to locate the xt-ldgenscripts.exe if needed.
3. Change the xtensa core if not using vp6_asic.
***

## Usage:
```
  xt-genldscripts [options]
Options are:
  -b lspDir         Specify LSP directory.
  -std lspName      Specify standard LSP directory by name. Sets <lspDir>
                     to <xtensa_root>/xtensa-elf/lib/<lspName> .
                     May not be combined with the -b option.   NOTE:  with the
                     -u or default -ldscripts options, the -std option modifies
                     the installed core package software, which is discouraged.
                     It is usually simpler and safer (e.g. for easier software
                     upgrades) to generate your own LSP.  The xt-regenlsps tool
                     simplifies this task. See LSP Reference Manual for details.
  -mref             Use the reference memory map as the main memory map.  The
                     reference map is built from processor configuration info
                     and reference options (-mvecbase, -mvecreset, and
                     -mvecselect) or their equivalent read from any specified
                     memory map parameters.
                     To use LSP's memmap.xmm parameters, you may also want -p.
  -m mapFile        Specify <mapFile> as the main memory map.  Always processed
                     before any -a, -r, or -ap options.  The default main
                     memory map (without -mref or -m) is <lspDir>/memmap.xmm
                     if -b or -std is specified, or the reference map otherwise.
  -a mapFile        Specify an additional memory map file.  Does not allow
                     redefining any memory, segment or section (by name).
                     May be specified multiple times.
  -r mapFile        Specify an additional memory map file.  Allows redefining
                     any memory, segment or section (by name).
                     May be specified multiple times.
  -ap mapFile       Specify an additional memory map file.  Memory descriptions
                     are ignored, only parameter assignments and other
                     directives are processed.  May be specified multiple times.
  -p                Like -ap <lspDir>/memmap.xmm, but processed before any -m.
  -mvecbase addr    Specify the expected run-time value of the VECBASE register
                     (same as parameter VECBASE=addr).
  -mvecselect 0|1   Specify the expected static vector select pin value
                     (same as parameter VECSELECT=n).
  -mvecreset addr   Specify the expected reset-time value of the alternate reset
                     vector (same as parameter VECRESET=n).
                    NOTE:  These three options imply -mref (they generate a new
                    memory map).  They also imply -p if -b or -std is specified.
                    The equivalent parameters don't have this effect.
  -mrompack         Pack RAM contents to ROM (same as ROMING=true).
  -mlocalmems       Put code and data in local memories by default
                    (ignored if core has no local instruction or data RAM).
  -o ldscriptfile   Output linker script in <ldscriptfile> instead of default.
  -reloc            For -o, output relocatable (incremental) linker script.
  -noldscripts      Don't output linker scripts in <lspDir>/ldscripts.
  -u                Update: shorthand for -map <lspDir>/memmap.xmm
                    (this option is generally recommended).
  -map mapOut       Output resulting memory map to file <mapOut>.
  -genmap mapOut    Older form of -map; implies -noldscripts and -memsonly.
  -memsonly         Skip non-memory elements in memory map output.
  -defer            Defer to link-time certain errors that commonly occur when
                     regenerating all standard LSPs, issuing a warning instead.
                     If a linker script is generated, it causes the linker to
                     report the error.  Applies to the following errors:
                     * ROMING=true specified but .rom.store section is missing
                     * RESERVE_SEGMENT_AREA or USE_SEGMENT_AREA specifies
                       a size that is too large for the segment
                     * PLACE to segment which doesn't exist
                     * INCLUDE_XTBOARD_MEMORIES=true but map overlaps xtboard devs
                     * MEMORY MAXSIZE is too small to contain required vectors
  -v                Slightly more verbose operation.
  -q                Omit output other than errors and warnings and -v additions.
  -help             Show this usage description and exit.
  -helpold          Show deprecated usage description and exit.
  --xtensa-core=<corename>         Specify Xtensa core/processor configuration.
  --xtensa-system=<dir>[:<dir>]*   Specify Xtensa core registry(ies).

Please refer to the Xtensa Linker Support Packages (LSP) Reference Manual for more information about using xt-genldscripts and xt-regenlsps.
```