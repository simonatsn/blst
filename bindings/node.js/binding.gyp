{
  'targets': [
    {
      'target_name': 'blst',
      'sources': [
        '<(INTERMEDIATE_DIR)/blst_wrap.cpp',
        '../../src/server.c',
      ],
      'include_dirs': [ '..' ],
      'cflags_cc': [ '-fexceptions' ],
      'xcode_settings': { 'GCC_ENABLE_CPP_EXCEPTIONS': 'YES' },
      'msvs_settings':  { 'VCCLCompilerTool': { 'ExceptionHandling': '1' } },
      'conditions': [
        [ 'OS=="win"', {
            'sources': [ '../../build/win64/*-x86_64.asm' ],
          }, {
            'sources': [ '../../build/assembly.S' ],
          }
        ],
        [ 'OS=="linux"', {
            'ldflags': [ '-Wl,-Bsymbolic' ],
          }
        ],
      ],
      'actions' : [
        {
          'action_name': 'blst_wrap',
          'variables': { 'cmd' : [ 'blst_wrap.py', '../blst.swg' ] },
          'inputs':  [ '<@(cmd)' ],
          'outputs': [ '<(INTERMEDIATE_DIR)/blst_wrap.cpp' ],
          'action':  [ 'python', '<@(cmd)', '<@(_outputs)' ],
        },
      ],
    },
  ]
}
