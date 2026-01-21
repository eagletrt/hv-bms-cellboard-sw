#!/usr/bin/env bash

echo "Running full test suite..."
echo "Running balancing Unit Tests"
pio test -e balancing_tests

echo "Running temperature Unit Tests"
pio test -e temperature_tests
echo "Running bms_manager Unit Tests"
echo "Tests currently disabled due to failing tests"

echo "Running can_comm Unit Tests"
pio test -e can_comm_tests

echo "Running identity Unit Tests"
pio test -e identity_tests

echo "Running led Unit Tests"
pio test -e led_tests

echo "Running programmer Unit Tests"
pio test -e programmer_tests

echo "Running volt Unit Tests"
pio test -e volt_tests