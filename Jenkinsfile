pipeline {
  agent none
  stages {
    stage('Compile') {
      steps {
        parallel(
          "OpenBSD 6.1": {
	    node('openbsd-6.1') {
		sh 'git config --global user.name "jenkins@kakwa.fr"'
        	sh 'git config --global user.email "jenkins@kakwa.fr"'
                git 'https://github.com/kakwa/uts-server'
                sh 'git clean -fdx'
                sh 'export CC=/usr/local/bin/egcc;export CXX=/usr/local/bin/ec++; cmake . -DBUNDLE_CIVETWEB=ON'
                sh 'export CC=/usr/local/bin/egcc;export CXX=/usr/local/bin/ec++; make -j4'
                sh './tests/cfg/pki/create_tsa_certs'
                sh './tests/external_test.sh'
            } 
          },
          "FreeBSD 11": {
	    node('freebsd-11') {
		sh 'git config --global user.email "jenkins@kakwa.fr"'
		sh 'git config --global user.name "jenkins@kakwa.fr"'
                git 'https://github.com/kakwa/uts-server'
                sh 'git clean -fdx'
                sh 'cmake . -DBUNDLE_CIVETWEB=ON'
                sh 'make -j4'
                sh './tests/cfg/pki/create_tsa_certs'
                sh './tests/external_test.sh'
            } 
          },
          "CentOS 7": {
	    node('centos-7') {
		sh 'git config --global user.email "jenkins@kakwa.fr"'
		sh 'git config --global user.name "jenkins@kakwa.fr"'
                git 'https://github.com/kakwa/uts-server'
                sh 'git clean -fdx'
                sh 'export CXX=/usr/bin/clang++; export CC=/usr/bin/clang; cmake . -DBUNDLE_CIVETWEB=ON'
                sh 'export CXX=/usr/bin/clang++; export CC=/usr/bin/clang; make -j4'
                sh './tests/cfg/pki/create_tsa_certs'
                sh './tests/external_test.sh'
            } 
          },
          "Debian 8": {
	    node('debian-8') {
		sh 'git config --global user.email "jenkins@kakwa.fr"'
		sh 'git config --global user.name "jenkins@kakwa.fr"'
                git 'https://github.com/kakwa/uts-server'
                sh 'git clean -fdx'
                sh 'cmake . -DBUNDLE_CIVETWEB=ON'
                sh 'make -j4'
                sh './tests/cfg/pki/create_tsa_certs'
                sh './tests/external_test.sh'
            } 
          }
        )
      }
    }
  }
}
