pipeline {
  agent none
  stages {
    stage('Compile') {
      steps {
        parallel(
          "OpenBSD 6.1": {
	    node('openbsd-6.1') {
                git 'https://github.com/kakwa/uts-server'
                sh 'CC=/usr/local/bin/egcc CXX=/usr/local/bin/ec++ cmake . -DBUNDLE_CIVETWEB=ON'
                sh 'make'
            } 
          },
          "FreeBSD 11": {
	    node('freebsd-11') {
                git 'https://github.com/kakwa/uts-server'
                sh 'cmake . -DBUNDLE_CIVETWEB=ON'
                sh 'make'
            } 
          },
          "CentOS 7": {
	    node('centos-7') {
                git 'https://github.com/kakwa/uts-server'
                sh 'cmake . -DBUNDLE_CIVETWEB=ON'
                sh 'make'
            } 
          },
          "Debian 8": {
	    node('debian-8') {
                git 'https://github.com/kakwa/uts-server'
                sh 'cmake . -DBUNDLE_CIVETWEB=ON'
                sh 'make'
            } 
          }
        )
      }
    }
  }
}
