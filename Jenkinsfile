pipeline {
  agent any
  stages {
    stage('error') {
      steps {
        parallel(
          "openbsd-6.1": {
            git 'https://github.com/kakwa/uts-server'
            sh 'cmake . -DBUNDLE_CIVETWEB=ON'
            sh 'make'
            
          },
          "freebsd-11": {
            git 'https://github.com/kakwa/uts-server'
            sh 'cmake . -DBUNDLE_CIVETWEB=ON'
            sh 'make'
            
          },
          "centos-7": {
            git 'https://github.com/kakwa/uts-server'
            sh 'cmake . -DBUNDLE_CIVETWEB=ON'
            sh 'make'
            
          },
          "debian-8": {
            git 'https://github.com/kakwa/uts-server'
            sh 'cmake . -DBUNDLE_CIVETWEB=ON'
            sh 'make'
            
          }
        )
      }
    }
  }
}