" Register the vimgcov extension with maktaba's coverage extension
let s:registry = maktaba#extension#GetRegistry('coverage')

" Add vimgcov extension to the registry
call s:registry.AddExtension({
      \ 'name': 'vimgcov',
      \ 'GetCoverage': function('vimgcov#GetCoverage'),
      \ 'IsAvailable': function('vimgcov#IsAvailable')
      \ })
