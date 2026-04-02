#!/bin/bash

# Basic test script for quip shell

echo "Testing quip shell..."

# Test 1: Help command
echo "Test 1: Help command"
echo "help" | ./quip | grep -q "Available builtins"
if [ $? -eq 0 ]; then
    echo "✓ Help command works"
else
    echo "✗ Help command failed"
fi

# Test 2: Built-in commands
echo "Test 2: Built-in commands"
echo -e "pwd\necho test\nexit" | ./quip | grep -q "test"
if [ $? -eq 0 ]; then
    echo "✓ Built-in commands work"
else
    echo "✗ Built-in commands failed"
fi

# Test 3: External commands
echo "Test 3: External commands"
echo -e "echo hello from external\nexit" | ./quip | grep -q "hello from external"
if [ $? -eq 0 ]; then
    echo "✓ External commands work"
else
    echo "✗ External commands failed"
fi

# Test 4: History
echo "Test 4: History functionality"
echo -e "echo test1\nhistory\nexit" | ./quip | grep -q "test1"
if [ $? -eq 0 ]; then
    echo "✓ History works"
else
    echo "✗ History failed"
fi

# Test 5: I/O Redirection
echo "Test 5: I/O Redirection"
echo "echo redirected output > test_output.txt" | ./quip
if [ -f "test_output.txt" ] && grep -q "redirected output" test_output.txt; then
    echo "✓ I/O redirection works"
    rm -f test_output.txt
else
    echo "✗ I/O redirection failed"
fi

# Test 6: Job control
echo "Test 6: Job control"
echo -e "sleep 1 &\njobs\nexit" | timeout 3 ./quip | grep -q "sleep"
if [ $? -eq 0 ]; then
    echo "✓ Job control works"
else
    echo "✗ Job control failed"
fi

echo "Basic tests completed."
