pipeline {
  agent any
  stages {
    stage('error') {
      steps {
        parallel(
          "OpenBSD": {
            git 'https://github.com/kakwa/uts-server'
            sh 'cmake . -DBUNDLE_CIVETWEB=ON'
            sh 'make'
            
          },
          "FreeBSD": {
            git 'https://github.com/kakwa/uts-server'
            sh 'cmake . -DBUNDLE_CIVETWEB=ON'
            sh 'make'
            
          },
          "CentOS 7": {
            git 'https://github.com/kakwa/uts-server'
            sh 'cmake . -DBUNDLE_CIVETWEB=ON'
            sh 'make'
            
          },
          "Debian 8": {
            git 'https://github.com/kakwa/uts-server'
            sh 'cmake . -DBUNDLE_CIVETWEB=ON'
            sh 'make'
            
          }
        )
      }
    }
  }
}