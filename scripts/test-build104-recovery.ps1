$ErrorActionPreference = 'Stop'
Push-Location (Join-Path $PSScriptRoot '..')
$build104Process = $null
$build104OldEnvironment = @{}
function Invoke-TestDocker {
    # Windows PowerShell 5 treats Docker's normal stderr progress as ErrorRecords.
    $ErrorActionPreference = 'Continue'
    $result = & docker @args 2>&1
    if ($LASTEXITCODE -ne 0) { throw ($result -join "`n") }
    $result | ForEach-Object { $_.ToString() }
}
try {
    New-Item -ItemType Directory -Path 'backend/logs' -Force | Out-Null
    if (!(Test-Path 'backend/target/swiftletcare-backend-0.0.1-SNAPSHOT.jar')) {
        throw 'Build the JAR with mvn -f backend/pom.xml clean verify first.'
    }
    Invoke-TestDocker compose -p swiftletcare-build104-e2e -f compose.build104-test.yml up -d --wait
    if ($LASTEXITCODE -ne 0) { throw 'Test services failed to start' }
    $exists = Invoke-TestDocker compose -p swiftletcare-build104-e2e -f compose.build104-test.yml exec -T postgres psql -U build104 -d build104_test -At -c "select 1 from pg_database where datname='build104_smoke'"
    if ($LASTEXITCODE -ne 0) { throw 'Cannot inspect test database' }
    if ($exists -ne '1') {
        Invoke-TestDocker compose -p swiftletcare-build104-e2e -f compose.build104-test.yml exec -T postgres createdb -U build104 build104_smoke
        if ($LASTEXITCODE -ne 0) { throw 'Cannot create smoke database' }
    }
    $build104Environment = @{
        DB_URL='jdbc:postgresql://127.0.0.1:55432/build104_smoke'
        DB_USERNAME='build104'; DB_PASSWORD='build104-test-only'
        JWT_SECRET='build104-smoke-secret-at-least-32-bytes'
        AI_SERVICE_KEY='build104-smoke-service-key-at-least-24'
        MQTT_URL='tcp://127.0.0.1:51883'; MQTT_CLIENT_ID='build104-jar-smoke'
        SERVER_PORT='58080'; SPRING_PROFILES_ACTIVE='local'
        LOCAL_ADMIN_PASSWORD='smoke-test-password'; DEBUG='false'
        HEARTBEAT_TIMEOUT_MS='6000'; STATUS_SCAN_MS='1000'; MQTT_ENABLED='true'
    }
    foreach ($name in $build104Environment.Keys) {
        $build104OldEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [Environment]::SetEnvironmentVariable($name, $build104Environment[$name], 'Process')
    }
    $build104Process = Start-Process -FilePath java -WindowStyle Hidden -PassThru `
        -ArgumentList @('-jar','backend/target/swiftletcare-backend-0.0.1-SNAPSHOT.jar') `
        -RedirectStandardOutput backend/logs/build104-jar.log -RedirectStandardError backend/logs/build104-jar-error.log
    foreach ($name in $build104OldEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $build104OldEnvironment[$name], 'Process')
    }
    $ready = $false
    $deadline = [DateTime]::UtcNow.AddSeconds(90)
    while ([DateTime]::UtcNow -lt $deadline) {
        if ($build104Process.HasExited) { throw 'Smoke JAR exited; inspect backend/logs/build104-jar.log' }
        try {
            $health = Invoke-RestMethod -TimeoutSec 3 http://127.0.0.1:58080/actuator/health/readiness
            if ($health.status -eq 'UP') { $ready = $true; break }
        } catch { }
        Start-Sleep -Milliseconds 300
    }
    if (!$ready) { throw 'Smoke JAR did not become ready' }
    python -u scripts/build104-smoke.py
    if ($LASTEXITCODE -ne 0) { throw 'Recovery smoke failed' }
} finally {
    if ($null -ne $build104Process -and !$build104Process.HasExited) {
        Stop-Process -Id $build104Process.Id
    }
    foreach ($name in $build104OldEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $build104OldEnvironment[$name], 'Process')
    }
    Pop-Location
}
