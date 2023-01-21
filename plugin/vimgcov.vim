let s:registry = maktaba#extension#GetRegistry('coverage')
call s:registry.AddExtension({
    \ 'name': 'vimgcov',
    \ 'GetCoverage': function('vimgcov#GetCoverage'),
    \ 'IsAvailable': function('vimgcov#IsAvailable')})
