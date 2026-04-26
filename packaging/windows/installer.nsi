; Gloubiboulga VST3 Installer for Windows
; Usage: makensis -DVERSION=1.0.0 installer.nsi
;
; Expects Gloubiboulga.vst3 to be present in the same directory as this script.

!define APPNAME    "Gloubiboulga"
!define PUBLISHER  "Fx-Mechanics"
!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"

Name            "${APPNAME} ${VERSION}"
OutFile         "Gloubiboulga-Windows-Setup.exe"
InstallDir      "$PROGRAMFILES64\Fx-Mechanics\${APPNAME}"
RequestExecutionLevel admin

Page instfiles

; ── Install ──────────────────────────────────────────────────────────────────
Section "VST3 Plugin"
    ; Copy the VST3 bundle to the standard system-wide VST3 directory
    SetOutPath "$COMMONFILES64\VST3"
    File /r "Gloubiboulga.vst3"

    ; Write uninstaller to a dedicated program directory
    SetOutPath "$INSTDIR"
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Register with Windows Add/Remove Programs
    WriteRegStr   HKLM "${UNINST_KEY}" "DisplayName"     "${APPNAME}"
    WriteRegStr   HKLM "${UNINST_KEY}" "Publisher"       "${PUBLISHER}"
    WriteRegStr   HKLM "${UNINST_KEY}" "DisplayVersion"  "${VERSION}"
    WriteRegStr   HKLM "${UNINST_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify"        1
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair"        1
SectionEnd

; ── Uninstall ─────────────────────────────────────────────────────────────────
Section "Uninstall"
    RMDir /r "$COMMONFILES64\VST3\Gloubiboulga.vst3"
    Delete   "$INSTDIR\Uninstall.exe"
    RMDir    "$INSTDIR"
    RMDir    "$PROGRAMFILES64\Fx-Mechanics"
    DeleteRegKey HKLM "${UNINST_KEY}"
SectionEnd
