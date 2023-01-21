let s:plugin = maktaba#plugin#Get('vimgcov')

let s:imported_python = 0

function! vimgcov#GetCoverage(filename) abort
    if !s:imported_python
        call maktaba#python#ImportModule(s:plugin, 'vimgcov')
        let s:imported_python = 1
    endif
    let l:coverage_data = maktaba#python#Eval(printf(
        \ 'vimgcov.GetCoverageGcovLines(%s)',
        \ string(a:filename)))
    let [l:covered_lines, l:uncovered_lines] = l:coverage_data
    return coverage#CreateReport(l:covered_lines, l:uncovered_lines, [])
endfunction

function! vimgcov#IsAvailable(unused_filename) abort
    return &filetype is# 'cpp' || &filetype is# 'c'
endfunction
