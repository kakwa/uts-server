pipeline {
  agent any
  stages {
    stage('error') {
      steps {
        parallel(
          "OpenBSD": {
            git 'https://github.com/kakwa/uts-server'
            
          },
          "FreeBSD": {
            git 'https://github.com/kakwa/uts-server'
            
          },
          "CentOS 7": {
            git 'https://github.com/kakwa/uts-server'
            
          },
          "Debian 8": {
            git 'https://github.com/kakwa/uts-server'
            
          }
        )
      }
    }
  }
}