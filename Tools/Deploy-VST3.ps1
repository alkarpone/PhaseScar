# Copies the freshly built PHASE SCAR VST3 bundle into the shared VST3 folder.
#
# Usage (from an elevated PowerShell):
#   .\Tools\Deploy-VST3.ps1                 # Debug build
#   .\Tools\Deploy-VST3.ps1 -Configuration Release
#
# Target layout:
#   C:\Program Files\Common Files\VST3\PHASE SCAR\PHASE SCAR <version>\<bundle>.vst3

[CmdletBinding()]
param(
		[ValidateSet('Debug', 'Release')]
		[string] $Configuration = 'Debug',

		[string] $ProjectRoot = (Split-Path -Parent $PSScriptRoot),

		[string] $VstRoot = 'C:\Program Files\Common Files\VST3\PHASE SCAR'
)

$ErrorActionPreference = 'Stop'

$definesFile = Join-Path $ProjectRoot 'JuceLibraryCode\JucePluginDefines.h'
if (-not (Test-Path $definesFile)) {
		throw "JucePluginDefines.h not found at '$definesFile'. Resave the .jucer file first."
}

$definesText = Get-Content $definesFile -Raw
if ($definesText -notmatch '#define\s+JucePlugin_VersionString\s+"([^"]+)"') {
		throw 'Could not read JucePlugin_VersionString from JucePluginDefines.h.'
}
$version = $Matches[1]

$buildDir = Join-Path $ProjectRoot "Builds\VisualStudio2026\x64\$Configuration\VST3"
$bundle = Get-ChildItem -Path $buildDir -Filter '*.vst3' -Directory -ErrorAction SilentlyContinue |
					Sort-Object LastWriteTime -Descending |
					Select-Object -First 1

if (-not $bundle) {
		throw "No .vst3 bundle found in '$buildDir'. Build the $Configuration configuration first."
}

$targetFolder = Join-Path $VstRoot "PHASE SCAR $version"
$targetBundle = Join-Path $targetFolder $bundle.Name

Write-Host "Source : $($bundle.FullName)"
Write-Host "Target : $targetBundle"

New-Item -ItemType Directory -Path $targetFolder -Force | Out-Null

if (Test-Path $targetBundle) {
		Remove-Item -Path $targetBundle -Recurse -Force
}

Copy-Item -Path $bundle.FullName -Destination $targetFolder -Recurse -Force

Write-Host "Deployed PHASE SCAR $version ($Configuration) successfully." -ForegroundColor Green
