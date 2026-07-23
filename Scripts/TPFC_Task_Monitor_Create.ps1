# TPFC Monitor Setup Script - Fixed Version
# This script creates the monitor script and sets up the Task Scheduler task

param(
    [string]$ProcessName = "TPFanControl",
    [int]$MaxRetries = 2,
    [int]$RetryDelaySeconds = 5,
    [int]$CheckIntervalMinutes = 1
)

# Function to create the monitor script
function New-MonitorScript {
    $monitorScript = @"
# TPFC Monitor Script - Auto-generated
param(
    [string]`$ProcessName = "$ProcessName",
    [string]`$LogPath = "`$env:TEMP\TPFC_FanControlMonitor.log",
    [int]`$MaxRetries = $MaxRetries,
    [int]`$RetryDelaySeconds = $RetryDelaySeconds
)

function Write-Log {
    param([string]`$Message, [string]`$Color = "White")
    `$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    `$logEntry = "[`$timestamp] `$Message"
    Write-Host `$logEntry -ForegroundColor `$Color
    Add-Content -Path `$LogPath -Value `$logEntry
}

function Show-ErrorPopup {
    param([string]`$Message)
    
    Add-Type -AssemblyName System.Windows.Forms
    [System.Windows.Forms.MessageBox]::Show(
        `$Message,
        "TPFC Monitor Error",
        [System.Windows.Forms.MessageBoxButtons]::OK,
        [System.Windows.Forms.MessageBoxIcon]::Error
    )
}

function Start-ProcessWithVerification {
    param([string]`$Path, [string]`$Name, [int]`$Retries = `$MaxRetries)
    
    for (`$attempt = 1; `$attempt -le `$Retries; `$attempt++) {
        Write-Log "Attempt `$attempt of `$Retries to start `$Name..." "Yellow"
        
        try {
            # Start the process
            Start-Process -FilePath `$Path -ErrorAction Stop
            Write-Log "Process launch command executed" "Gray"
            
            # Wait for process to initialize
            Start-Sleep -Seconds `$RetryDelaySeconds
            
            # Verify process is running
            `$process = Get-Process -Name `$Name -ErrorAction SilentlyContinue
            if (`$process) {
                Write-Log "Successfully started `$Name (PID: `$(`$process.Id))" "Green"
                return `$process
            }
            
            # If not running, wait and retry
            if (`$attempt -lt `$Retries) {
                Write-Log "Process not detected, waiting `$RetryDelaySeconds seconds before retry..." "Yellow"
                Start-Sleep -Seconds `$RetryDelaySeconds
            }
        }
        catch {
            Write-Log "Attempt `$attempt failed: `$(`$_.Exception.Message)" "Red"
            if (`$attempt -eq `$Retries) {
                throw
            }
        }
    }
    
    return `$null
}

try {
    # Get the directory where this script is located
    `$ScriptDirectory = Split-Path -Parent `$MyInvocation.MyCommand.Path
    `$ProcessPath = Join-Path `$ScriptDirectory "`$ProcessName.exe"

    Write-Log "Monitoring `$ProcessName..." "Gray"

    # Check if process exists
    `$process = Get-Process -Name `$ProcessName -ErrorAction SilentlyContinue

    if (`$process) {
        Write-Log "`$ProcessName is running (PID: `$(`$process.Id))" "Green"
    }
    else {
        Write-Log "`$ProcessName is not running. Attempting to start..." "Yellow"
        
        # Verify the executable exists
        if (-not (Test-Path `$ProcessPath)) {
            `$errorMsg = "`$ProcessName.exe not found in: `$ScriptDirectory`n`nPlease make sure the executable exists in the same folder as this script."
            Write-Log "ERROR: `$errorMsg" "Red"
            Show-ErrorPopup `$errorMsg
            exit 1
        }
        
        # Try to start the process with verification
        `$newProcess = Start-ProcessWithVerification -Path `$ProcessPath -Name `$ProcessName -Retries `$MaxRetries
        
        if (-not `$newProcess) {
            `$errorMsg = "Failed to start `$ProcessName after `$MaxRetries attempts.`n`nExecutable: `$ProcessPath`n`nPlease check if the application is working properly."
            Write-Log "ERROR: `$errorMsg" "Red"
            Show-ErrorPopup `$errorMsg
        }
        else {
            # Log success
            try {
                if (Get-Command Write-EventLog -ErrorAction SilentlyContinue) {
                    Write-EventLog -LogName Application -Source "Application" -EventId 1001 -EntryType Information -Message "`$ProcessName process was started by monitor script. PID: `$(`$newProcess.Id)"
                }
            }
            catch {
                Write-Log "Note: Could not write to event log" "Gray"
            }
        }
    }
}
catch {
    `$errorMsg = "Unexpected error: `$(`$_.Exception.Message)"
    Write-Log "ERROR: `$errorMsg" "Red"
    Show-ErrorPopup `$errorMsg
}
"@
    return $monitorScript
}

# Function to create Task Scheduler task - FIXED VERSION
function New-ScheduledTask {
    param(
        [string]$ScriptPath,
        [int]$IntervalMinutes
    )
    
    $TaskName = "TPFC Monitor"
    $ScriptDir = Split-Path -Parent $ScriptPath
    
    # Check if task already exists
    $existingTask = Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue
    if ($existingTask) {
        Write-Host "Task '$TaskName' already exists. Unregistering..." -ForegroundColor Yellow
        Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false
    }
    
    # Create scheduled task action
    $action = New-ScheduledTaskAction -Execute "powershell.exe" -Argument "-WindowStyle Hidden -File `"$ScriptPath`""
    
    # Create triggers
    $triggers = @()
    
    # Trigger 1: At logon
    $logonTrigger = New-ScheduledTaskTrigger -AtLogOn -RandomDelay "00:01:00"
    $triggers += $logonTrigger
    
    # Trigger 2: Time-based repeating trigger (valid XML duration)
    $startTime = (Get-Date).AddMinutes(1)
    $repeatTrigger = New-ScheduledTaskTrigger -Once -At $startTime `
        -RepetitionInterval (New-TimeSpan -Minutes $IntervalMinutes) `
        -RepetitionDuration (New-TimeSpan -Days 3650)   # 10 years
    $triggers += $repeatTrigger
    
    # Task settings
    $settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -StartWhenAvailable -RestartCount 3 -RestartInterval (New-TimeSpan -Minutes 1)
    
    # Task principal
    $principal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -LogonType ServiceAccount -RunLevel Highest
    
    try {
        $task = Register-ScheduledTask -TaskName $TaskName -Action $action -Trigger $triggers -Settings $settings -Principal $principal -Description "Monitors and restarts TPFanControl if not running. Checks every $IntervalMinutes minutes." -Force
        Write-Host "✓ Scheduled task '$TaskName' created successfully!" -ForegroundColor Green
        Write-Host "  - Runs at logon" -ForegroundColor Gray
        Write-Host "  - Starts 1 minutes after creation" -ForegroundColor Gray
        Write-Host "  - Repeats every $IntervalMinutes minutes (for 10 years)" -ForegroundColor Gray
        Write-Host "  - Runs with highest privileges" -ForegroundColor Gray
        return $true
    }
    catch {
        Write-Host "✗ Failed to create scheduled task: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "Trying alternative method..." -ForegroundColor Yellow
        return New-ScheduledTaskAlternative -ScriptPath $ScriptPath -IntervalMinutes $IntervalMinutes
    }
}



# Alternative method using schtasks command
function New-ScheduledTaskAlternative {
    param(
        [string]$ScriptPath,
        [int]$IntervalMinutes
    )
    
    $TaskName = "FanControlMonitor"
    $XMLPath = Join-Path $env:TEMP "FanControlTask.xml"
    
    # Create task XML manually
    $taskXML = @"
<?xml version="1.0" encoding="UTF-16"?>
<Task version="1.2" xmlns="http://schemas.microsoft.com/windows/2004/02/mit/task">
  <RegistrationInfo>
    <Description>Monitors and restarts TPFanControl if not running. Checks every $IntervalMinutes minutes.</Description>
  </RegistrationInfo>
  <Triggers>
    <LogonTrigger>
      <Enabled>true</Enabled>
      <RandomDelay>PT1M</RandomDelay>
    </LogonTrigger>
    <CalendarTrigger>
      <StartBoundary>$(Get-Date -Format "yyyy-MM-ddTHH:mm:ss")</StartBoundary>
      <Enabled>true</Enabled>
      <ScheduleByDay>
        <DaysInterval>1</DaysInterval>
      </ScheduleByDay>
      <Repetition>
        <Interval>PT${IntervalMinutes}M</Interval>
        <Duration>P1D</Duration>
        <StopAtDurationEnd>false</StopAtDurationEnd>
      </Repetition>
    </CalendarTrigger>
  </Triggers>
  <Principals>
    <Principal id="Author">
      <UserId>SYSTEM</UserId>
      <LogonType>ServiceAccount</LogonType>
      <RunLevel>HighestAvailable</RunLevel>
    </Principal>
  </Principals>
  <Settings>
    <MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy>
    <DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>
    <StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>
    <AllowHardTerminate>true</AllowHardTerminate>
    <StartWhenAvailable>true</StartWhenAvailable>
    <RunOnlyIfNetworkAvailable>false</RunOnlyIfNetworkAvailable>
    <IdleSettings>
      <StopOnIdleEnd>true</StopOnIdleEnd>
      <RestartOnIdle>false</RestartOnIdle>
    </IdleSettings>
    <AllowStartOnDemand>true</AllowStartOnDemand>
    <Enabled>true</Enabled>
    <Hidden>false</Hidden>
    <RunOnlyIfIdle>false</RunOnlyIfIdle>
    <WakeToRun>false</WakeToRun>
    <ExecutionTimeLimit>PT1H</ExecutionTimeLimit>
    <Priority>7</Priority>
  </Settings>
  <Actions Context="Author">
    <Exec>
      <Command>powershell.exe</Command>
      <Arguments>-WindowStyle Hidden -File "$ScriptPath"</Arguments>
    </Exec>
  </Actions>
</Task>
"@
    
    try {
        # Save XML to file
        $taskXML | Out-File -FilePath $XMLPath -Encoding Unicode
        
        # Delete task if it exists
        schtasks /delete /tn $TaskName /f 2>$null
        
        # Create task using schtasks
        $result = schtasks /create /tn $TaskName /xml $XMLPath /f
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ Scheduled task '$TaskName' created successfully (using alternative method)!" -ForegroundColor Green
            Remove-Item $XMLPath -ErrorAction SilentlyContinue
            return $true
        } else {
            throw "schtasks failed with exit code $LASTEXITCODE"
        }
    }
    catch {
        Write-Host "✗ Alternative method also failed: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "You may need to create the task manually in Task Scheduler." -ForegroundColor Yellow
        return $false
    }
}

# Main setup execution
Write-Host "=== TPFC Monitor Setup ===" -ForegroundColor Cyan
Write-Host "This script will:" -ForegroundColor White
Write-Host "1. Create the monitor PowerShell script in the current directory" -ForegroundColor White
Write-Host "2. Set up a Windows Task Scheduler task to run it automatically" -ForegroundColor White
Write-Host "" 

# Get current directory
$CurrentDir = Get-Location
$MonitorScriptPath = Join-Path $CurrentDir.Path "Monitor-FanControl.ps1"

Write-Host "Current directory: $CurrentDir" -ForegroundColor Gray
Write-Host "Monitor script will be created at: $MonitorScriptPath" -ForegroundColor Gray
Write-Host ""

# Check if TPFanControl.exe exists in current directory
if (Test-Path "TPFanControl.exe") {
    Write-Host "✓ TPFanControl.exe found in current directory" -ForegroundColor Green
} else {
    Write-Host "⚠ WARNING: TPFanControl.exe not found in current directory" -ForegroundColor Yellow
    Write-Host "  Make sure to place this setup script in the same folder as TPFanControl.exe" -ForegroundColor Yellow
}

Write-Host ""
$confirmation = Read-Host "Proceed with setup? (Y/N)"
if ($confirmation -notmatch '^[Yy]') {
    Write-Host "Setup cancelled." -ForegroundColor Yellow
    exit
}

# Create the monitor script
Write-Host "`nCreating monitor script..." -ForegroundColor Yellow
$monitorScriptContent = New-MonitorScript
try {
    $monitorScriptContent | Out-File -FilePath $MonitorScriptPath -Encoding UTF8
    Write-Host "✓ Monitor script created successfully at: $MonitorScriptPath" -ForegroundColor Green
}
catch {
    Write-Host "✗ Failed to create monitor script: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Create the scheduled task
Write-Host "`nCreating scheduled task..." -ForegroundColor Yellow
if (New-ScheduledTask -ScriptPath $MonitorScriptPath -IntervalMinutes $CheckIntervalMinutes) {
    Write-Host "`n=== Setup Complete! ===" -ForegroundColor Green
    Write-Host "✓ Monitor script: $MonitorScriptPath" -ForegroundColor Green
    Write-Host "✓ Scheduled task: 'TPFC Monitor'" -ForegroundColor Green
    Write-Host "✓ Check interval: Every $CheckIntervalMinutes minutes" -ForegroundColor Green
    Write-Host "✓ Max retries: $MaxRetries" -ForegroundColor Green
    Write-Host ""
    Write-Host "The monitor will:" -ForegroundColor White
    Write-Host "- Start automatically when you log in" -ForegroundColor Gray
    Write-Host "- Check every $CheckIntervalMinutes minutes if TPFanControl is running" -ForegroundColor Gray
    Write-Host "- Restart it automatically if needed" -ForegroundColor Gray
    Write-Host "- Show error popups if it can't start the process" -ForegroundColor Gray
    Write-Host ""
    Write-Host "You can manually run the monitor script to test it:" -ForegroundColor White
    Write-Host "powershell -File `"$MonitorScriptPath`"" -ForegroundColor Gray
} else {
    Write-Host "`n⚠ Setup partially complete." -ForegroundColor Yellow
    Write-Host "  Monitor script was created, but scheduled task setup failed." -ForegroundColor Yellow
    Write-Host "  You can manually run the script or set up the task in Task Scheduler." -ForegroundColor Yellow
}

# Test the script if user wants
Write-Host ""
$test = Read-Host "Would you like to test the monitor script now? (Y/N)"
if ($test -match '^[Yy]') {
    Write-Host "Testing monitor script..." -ForegroundColor Yellow
    & "powershell.exe" -File "`"$MonitorScriptPath`""
}
