# TPFC Task Monitor Cleanup Script
# This script removes the monitor script and Task Scheduler task

param(
    [string]$TaskName = "TPFC Monitor",
    [string]$MonitorScriptName = "TPFC_Monitor.ps1",
    [switch]$Force
)

function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

function Confirm-Action {
    param([string]$Message)
    
    if ($Force) {
        return $true
    }
    
    $confirmation = Read-Host "$Message (Y/N)"
    return $confirmation -match '^[Yy]'
}

Write-Host "=== TPFC Monitor Cleanup ===" -ForegroundColor Cyan
Write-Host "This script will remove:" -ForegroundColor White
Write-Host "1. The monitor PowerShell script" -ForegroundColor White
Write-Host "2. The Task Scheduler task" -ForegroundColor White
Write-Host ""

# Get current directory
$CurrentDir = Get-Location
$MonitorScriptPath = Join-Path $CurrentDir.Path $MonitorScriptName

Write-Host "Current directory: $CurrentDir" -ForegroundColor Gray
Write-Host "Monitor script location: $MonitorScriptPath" -ForegroundColor Gray
Write-Host "Task name: $TaskName" -ForegroundColor Gray
Write-Host ""

if (-not $Force) {
    $proceed = Confirm-Action "Proceed with cleanup?"
    if (-not $proceed) {
        Write-ColorOutput "Cleanup cancelled." "Yellow"
        exit
    }
}

# Remove Task Scheduler task
Write-Host "`nRemoving Task Scheduler task..." -ForegroundColor Yellow
try {
    $existingTask = Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue
    if ($existingTask) {
        if ($Force -or (Confirm-Action "Delete scheduled task '$TaskName'?")) {
            Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false
            Write-ColorOutput "✓ Scheduled task '$TaskName' removed successfully!" "Green"
        } else {
            Write-ColorOutput "⏭ Skipped task removal" "Yellow"
        }
    } else {
        Write-ColorOutput "ℹ Scheduled task '$TaskName' not found" "Gray"
    }
}
catch {
    Write-ColorOutput "✗ Failed to remove scheduled task: $($_.Exception.Message)" "Red"
    
    # Try alternative method using schtasks
    Write-ColorOutput "Trying alternative removal method..." "Yellow"
    try {
        schtasks /delete /tn $TaskName /f 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-ColorOutput "✓ Scheduled task removed using alternative method!" "Green"
        } else {
            Write-ColorOutput "ℹ Task may have already been deleted" "Gray"
        }
    }
    catch {
        Write-ColorOutput "ℹ Could not remove task (may not exist)" "Gray"
    }
}

# Remove monitor script
Write-Host "`nRemoving monitor script..." -ForegroundColor Yellow
if (Test-Path $MonitorScriptPath) {
    if ($Force -or (Confirm-Action "Delete monitor script '$MonitorScriptName'?")) {
        try {
            Remove-Item -Path $MonitorScriptPath -Force -ErrorAction Stop
            Write-ColorOutput "✓ Monitor script removed successfully!" "Green"
        }
        catch {
            Write-ColorOutput "✗ Failed to remove monitor script: $($_.Exception.Message)" "Red"
        }
    } else {
        Write-ColorOutput "⏭ Skipped script removal" "Yellow"
    }
} else {
    Write-ColorOutput "ℹ Monitor script not found at: $MonitorScriptPath" "Gray"
}

# Optional: Remove log file
Write-Host "`nCleaning up log files..." -ForegroundColor Yellow
$LogPath = "$env:TEMP\TPFC_FanControlMonitor.log"
if (Test-Path $LogPath) {
    if ($Force -or (Confirm-Action "Delete log file '$LogPath'?")) {
        try {
            Remove-Item -Path $LogPath -Force -ErrorAction SilentlyContinue
            Write-ColorOutput "✓ Log file removed successfully!" "Green"
        }
        catch {
            Write-ColorOutput "ℹ Could not remove log file: $($_.Exception.Message)" "Gray"
        }
    } else {
        Write-ColorOutput "⏭ Skipped log file removal" "Yellow"
    }
} else {
    Write-ColorOutput "ℹ Log file not found" "Gray"
}

Write-Host "`n=== Cleanup Complete! ===" -ForegroundColor Green

# Summary
Write-Host "`nSummary of actions taken:" -ForegroundColor White
$taskRemoved = -not (Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue)
$scriptRemoved = -not (Test-Path $MonitorScriptPath)

if ($taskRemoved) {
    Write-Host "✓ Task Scheduler task: REMOVED" -ForegroundColor Green
} else {
    Write-Host "✗ Task Scheduler task: STILL EXISTS" -ForegroundColor Red
}

if ($scriptRemoved) {
    Write-Host "✓ Monitor script: REMOVED" -ForegroundColor Green
} else {
    Write-Host "✗ Monitor script: STILL EXISTS" -ForegroundColor Red
}

if ($taskRemoved -and $scriptRemoved) {
    Write-Host "`n🎉 Full cleanup completed successfully!" -ForegroundColor Green
} else {
    Write-Host "`n⚠ Partial cleanup completed. Some items may need manual removal." -ForegroundColor Yellow
}
