# ensure sarif are in path
$env:PATH+="$([IO.Path]::PathSeparator)${HOME}/.cargo/bin/"

$extraArgs = @()

if($isLinux) {
    # the project was built wit gcc ensure craft clang finds the toolchain
    $extraArgs += @("-extra-arg=--gcc-toolchain=$($(Get-Item $(Get-command ${env:CXX}).Source).Directory.Parent)")
}

$clangCommand = $extraArgs + @("-p",  "$env:BUILD_DIR")

Write-Host "Running run-clang-tidy ${clangCommand}"
run-clang-tidy @clangCommand | clang-tidy-sarif  | Tee-Object -Path "${env:GITHUB_WORKSPACE}/clang-tidy.sarif" | sarif-fmt

$sarifFile = "${env:GITHUB_WORKSPACE}/clang-tidy.sarif"
$workspacePrefix = ($env:GITHUB_WORKSPACE -replace '\\', '/') + '/'

$sarif = Get-Content -Raw -Path $sarifFile | ConvertFrom-Json
foreach ($run in $sarif.runs) {
    foreach ($result in $run.results) {
        foreach ($loc in $result.locations) {
            if ($loc.physicalLocation.artifactLocation.uri) {
                $loc.physicalLocation.artifactLocation.uri = $loc.physicalLocation.artifactLocation.uri -replace "^$([regex]::Escape($workspacePrefix))", ""
            }
        }
    }
}
$sarif | ConvertTo-Json -Depth 100 | Set-Content -Path $sarifFile
