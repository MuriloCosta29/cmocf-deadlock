#!/usr/bin/env bash

{
  date
  whoami
  pwd
} >>evidencias.log

script -a evidencias.log bash
