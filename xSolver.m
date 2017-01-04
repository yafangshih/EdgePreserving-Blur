% 
% Implementation codes of Edge-preserving blur
% Ya-Fang Shih yfshih.tw[at]gmail.com
% Dec. 2016 released under the FreeBSD license
%

[filename, consts_len, N, M, ksize] = textread('dim.txt', '%s %d %d %d %d\n');
imgSize = N*M;

Emaxima = zeros(N, M, 1, 'double');
Eminima = zeros(N, M, 1, 'double');
EMean = zeros(N, M, 3, 'double');


for i=0:2
    [row_inds,col_inds,vals] = textread(sprintf('A%dM.txt', i), '%d %d %f\n');
    row_inds = row_inds +1 ;
    col_inds = col_inds +1 ;
    [b] = textread(sprintf('b%dM.txt', i), '%f\n');
    A=sparse(row_inds,col_inds,vals,consts_len,imgSize);
    new_vals=A\b; 
    Emaxima = transpose(reshape(new_vals, M, N));

    [row_inds,col_inds,vals] = textread(sprintf('A%dm.txt', i), '%d %d %f\n');
    row_inds = row_inds +1 ;
    col_inds = col_inds +1 ;
    [b] = textread(sprintf('b%dm.txt', i), '%f\n');
    A=sparse(row_inds,col_inds,vals,consts_len,imgSize);
    new_vals=A\b; 
    Eminima = transpose(reshape(new_vals, M, N));

    if i==0
        EMean(:,:,3) = (Eminima(:,:,1) + Emaxima(:,:,1))/2;
    end
    if i==1
        EMean(:,:,2) = (Eminima(:,:,1) + Emaxima(:,:,1))/2;
    end
    if i==2
        EMean(:,:,1) = (Eminima(:,:,1) + Emaxima(:,:,1))/2;
    end
    
    delete(sprintf('A%dM.txt', i));
    delete(sprintf('b%dM.txt', i));
    delete(sprintf('A%dm.txt', i));
    delete(sprintf('b%dm.txt', i));
end

delete('dim.txt');

f=sprintf('data/output/k%d%s', ksize, filename{1,1});
imwrite(EMean, f);

exit;
