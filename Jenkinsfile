pipeline {
  agent none
  environment {
    SVNCREDS = credentials('83601f39-7c4c-4a13-a0d3-59fcda340753')
  }
  stages {
    stage('Parallel build') {
      parallel {
        stage('Build on Mac Intel') {
          agent {
            label 'mac && x86 && build && 20.0'
          }
          steps {
            sh '''#!/bin/bash
              set -e
              echo "====== macOS: starting build"
              export BITS=64
              
              PLATFORM=mac  ./mk_kafka.sh $BITS
              echo "====== macOS: finished build"
            '''
            stash name: 'dist-mac', includes: 'distribution/mac/x64/'
          }
        }
        stage('Build on Mac Arm') {
          agent {
            label 'mac && arm && build && 20.0'
          }
          steps {
            sh '''#!/bin/bash
              set -e
              echo "====== macOS: starting build"
              export BITS=64 

              PLATFORM=mac  ./mk_kafka.sh $BITS
              echo "====== macOS: finished build"
            '''
            stash name: 'dist-mac_arm', includes: 'distribution/mac/arm64/'
          }
        }
        stage('Build on Linux') {
          agent {
            docker {
              image 'dyalogci/ubuntu:20.04-build'
              registryCredentialsId '0435817a-5f0f-47e1-9dcc-800d85e5c335'
              args '-v /devt:/devt'
            }
          }
          steps {
            sh '''#!/bin/bash
              set -e
              echo "====== Linux: starting build"
              export BITS=64 

              PLATFORM=linux ./mk_kafka.sh $BITS              
              echo "====== Linux: finished build"
            '''
            stash name: 'dist-linux', includes: 'distribution/linux/x64/'
          }
        }
        stage('Build on Windows') {
          agent {
            label 'win && build && 20.0'
          }
          steps {
            bat 'kafkaBuild.bat'
            stash name: 'dist-win', includes: 'distribution/win/'
          }
        }
      }
    }
    stage('Publish') {
          agent {
            docker {
              image 'dyalogci/ubuntu:20.04-build'
              registryCredentialsId '0435817a-5f0f-47e1-9dcc-800d85e5c335'
              args '-v /devt:/devt'
            }
          }
      steps {
        unstash 'dist-win'
        unstash 'dist-mac'
        unstash 'dist-mac_arm'
        unstash 'dist-linux'
        sh './publish.sh'
      }
    }
  }
}
