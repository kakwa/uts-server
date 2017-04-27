pipeline {
  agent none
  stages {
    stage('Compile') {
      steps {
        parallel(
          "OpenBSD 6.1": {
            node(label: 'openbsd-6.1') {
              sh 'git config --global user.name "jenkins@kakwa.fr"'
              sh 'git config --global user.email "jenkins@kakwa.fr"'
              git(url: 'https://github.com/kakwa/uts-server', poll: true, changelog: true)
              sh 'git clean -fdx'
              sh 'export GIT_SSL_NO_VERIFY=true; CIVETWEB_GITURL=https://gogs.kakwa.fr/kakwa/civetweb/; export CC=/usr/local/bin/egcc;export CXX=/usr/local/bin/ec++; cmake . -DBUNDLE_CIVETWEB=ON'
              sh 'export GIT_SSL_NO_VERIFY=true; CIVETWEB_GITURL=https://gogs.kakwa.fr/kakwa/civetweb/; export CC=/usr/local/bin/egcc;export CXX=/usr/local/bin/ec++; make -j4'
              sh './tests/cfg/pki/create_tsa_certs'
              sh './tests/external_test.sh'
            }
            
            
          },
          "FreeBSD 11": {
            node(label: 'freebsd-11') {
              sh 'git config --global user.email "jenkins@kakwa.fr"'
              sh 'git config --global user.name "jenkins@kakwa.fr"'
              git(url: 'https://github.com/kakwa/uts-server', poll: true, changelog: true)
              sh 'export GIT_SSL_NO_VERIFY=true; CIVETWEB_GITURL=https://gogs.kakwa.fr/kakwa/civetweb/;git clean -fdx'
              sh 'export GIT_SSL_NO_VERIFY=true; CIVETWEB_GITURL=https://gogs.kakwa.fr/kakwa/civetweb/;cmake . -DBUNDLE_CIVETWEB=ON'
              sh 'make -j4'
              sh './tests/cfg/pki/create_tsa_certs'
              sh './tests/external_test.sh'
            }
            
            
          },
          "CentOS 7": {
            node(label: 'centos-7') {
              sh 'git config --global user.email "jenkins@kakwa.fr"'
              sh 'git config --global user.name "jenkins@kakwa.fr"'
              git(url: 'https://github.com/kakwa/uts-server', poll: true, changelog: true)
              sh 'git clean -fdx'
              sh 'export GIT_SSL_NO_VERIFY=true; CIVETWEB_GITURL=https://gogs.kakwa.fr/kakwa/civetweb/; export CXX=/usr/bin/clang++; export CC=/usr/bin/clang; cmake . -DBUNDLE_CIVETWEB=ON'
              sh 'export GIT_SSL_NO_VERIFY=true; CIVETWEB_GITURL=https://gogs.kakwa.fr/kakwa/civetweb/; export CXX=/usr/bin/clang++; export CC=/usr/bin/clang; make -j4'
              sh './tests/cfg/pki/create_tsa_certs'
              sh './tests/external_test.sh'
            }
            
            
          },
          "Debian 8": {
            node(label: 'debian-8') {
              sh 'git config --global user.email "jenkins@kakwa.fr"'
              sh 'git config --global user.name "jenkins@kakwa.fr"'
              git(url: 'https://github.com/kakwa/uts-server', poll: true, changelog: true)
              sh 'git clean -fdx'
              sh 'export GIT_SSL_NO_VERIFY=true; CIVETWEB_GITURL=https://gogs.kakwa.fr/kakwa/civetweb/; cmake . -DBUNDLE_CIVETWEB=ON'
              sh 'export GIT_SSL_NO_VERIFY=true; CIVETWEB_GITURL=https://gogs.kakwa.fr/kakwa/civetweb/; make -j4'
              sh './tests/cfg/pki/create_tsa_certs'
              sh './tests/external_test.sh'
            }
            
            
          }
        )
      }
    }
  }
}