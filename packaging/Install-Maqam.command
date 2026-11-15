#!/bin/bash
# Installs the Maqam plugin for the current user.
cd "$(dirname "$0")"
AU="$HOME/Library/Audio/Plug-Ins/Components"
VST3="$HOME/Library/Audio/Plug-Ins/VST3"
mkdir -p "$AU" "$VST3"
rm -rf "$AU/Maqam.component" "$VST3/Maqam.vst3"
cp -R Maqam.component "$AU/"
cp -R Maqam.vst3 "$VST3/"
xattr -dr com.apple.quarantine "$AU/Maqam.component" "$VST3/Maqam.vst3" 2>/dev/null
killall -9 AudioComponentRegistrar 2>/dev/null
echo ""
echo "Maqam installed:"
echo "  AU   -> $AU/Maqam.component"
echo "  VST3 -> $VST3/Maqam.vst3"
echo "Restart your DAW. In Logic: MIDI FX slot > Audio Units > Maqam Tools > Maqam."
