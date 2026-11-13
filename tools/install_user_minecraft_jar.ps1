param(
    [Parameter(Mandatory = $true)]
    [string]$OptiCraftZip,

    [Parameter(Mandatory = $true)]
    [string]$SdRoot
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.IO.Compression.FileSystem

$zipPath = [System.IO.Path]::GetFullPath($OptiCraftZip)
$sdPath = [System.IO.Path]::GetFullPath($SdRoot)
if (-not (Test-Path -LiteralPath $zipPath -PathType Leaf)) {
    throw "ZIP archive not found: $zipPath"
}
if (-not (Test-Path -LiteralPath $sdPath -PathType Container)) {
    throw "SD card root not found: $sdPath"
}

$entryName = "OptiCraft JAVA LITE 1.2.5/mcdata/bin/minecraft.jar"
$archive = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
try {
    $entry = $archive.GetEntry($entryName)
    if ($null -eq $entry) {
        throw "minecraft.jar was not found inside the selected OptiCraft archive."
    }

    $targetDir = Join-Path $sdPath "switch\opticraft"
    New-Item -ItemType Directory -Force -Path $targetDir | Out-Null
    $targetFile = Join-Path $targetDir "minecraft.jar"

    $sourceStream = $entry.Open()
    try {
        $targetStream = [System.IO.File]::Open(
            $targetFile,
            [System.IO.FileMode]::Create,
            [System.IO.FileAccess]::Write,
            [System.IO.FileShare]::None
        )
        try {
            $sourceStream.CopyTo($targetStream)
        }
        finally {
            $targetStream.Dispose()
        }
    }
    finally {
        $sourceStream.Dispose()
    }

    Write-Host "Installed user's minecraft.jar to:"
    Write-Host $targetFile
}
finally {
    $archive.Dispose()
}
